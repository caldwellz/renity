/****************************************************
 * ScriptContext.cc: JavaScript execution context   *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/ScriptContext.h"

#include <SDL3/SDL_log.h>

#include "ActionManager.h"
#include "Dictionary.h"
#include "ResourceManager.h"
#include "resources/StringBuffer.h"
#include "utils/id_helpers.h"

namespace renity {
ScriptContext* currentScriptContext = nullptr;
/***** Auto-bound script helper functions *****/

// Debug logger
// TODO: Allow varying numbers of arguments
static duk_ret_t scriptConsoleLog(duk_context* ctx) {
  // Stringify objects so the string value is more than just "[object Object]"
  if (duk_is_object(ctx, -1)) {
    duk_json_encode(ctx, -1);
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "[Script] console.log: %s",
               duk_safe_to_string(ctx, -1));
  return 0;
}

// Module loader
static duk_ret_t scriptRequire(duk_context* ctx) {
  // Return an empty object in case of error
  duk_push_bare_object(ctx);  // [path] -> [path, {}]

  // Read and sanity-check the given path
  const char* path = duk_get_lstring(ctx, 0, nullptr);
  if (!path) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "[Script] require() invoked without a path string as the 1st param");
    return 1;
  }
  StringBufferPtr buf = ResourceManager::getActive()->get<StringBuffer>(path);
  if (!buf->length()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] require('%s'): Invalid path or empty file", path);
    return 1;
  }
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Attempting to require('%s')", path);

  // Compile the script as a global program, associating the real fileName
  duk_dup(ctx, 0);  // [path, {}] -> [path, {}, path]
  duk_int_t result = duk_pcompile_lstring_filename(
      ctx, DUK_COMPILE_STRICT, buf->getCStr(), buf->length());
  if (result != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] require('%s'): Compilation failed with '%s'", path,
                 duk_safe_to_string(ctx, -1));
    duk_pop(ctx);  // [path, {}, err] -> [path, {}]
    return 1;
  }

  // Replace the global module.exports with a new empty object
  // Users of previous requires should still hold references to the old object
  duk_push_global_object(ctx);  // [path, {}, func] -> [..., global]
  duk_push_bare_object(ctx);
  duk_push_bare_object(ctx);                // -> [..., global, module, exports]
  duk_put_prop_string(ctx, -2, "exports");  // exports -> module.exports
  duk_put_prop_string(ctx, -2, "module");   // module -> global.module
  duk_pop(ctx);                             // -> [path, {}, func]

  // Execute the compiled script
  result = duk_pcall(ctx, 0);  // -> [path, {}, retval | err]
  if (result != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] require('%s'): Execution failed with '%s'", path,
                 duk_safe_to_string(ctx, -1));
    duk_pop(ctx);  // [path, {}, err] -> [path, {}]
    return 1;
  }

  // Ignore the automatic return value and return module.exports to the caller
  duk_push_global_object(ctx);  // -> [..., global]
  if (!duk_get_prop_string(ctx, -1, "module") ||
      !duk_get_prop_string(ctx, -1, "exports")) {
    duk_set_top(ctx, 1);  // [..., global, module?] -> [path, {}]
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "[Script] require('%s'): module.exports missing after execution", path);
  }
  return 1;  // [..., global, module, exports]
}

// Actions.assignCategory
static duk_ret_t scriptAssignCategory(duk_context* ctx) {
  if (!duk_is_string(ctx, 0) || !duk_is_string(ctx, 1)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Actions.assignCategory: Invalid parameter type(s)");
    duk_push_uint(ctx, 0);
    return 1;
  }

  const char* actionName = duk_get_string(ctx, 0);
  const char* categoryName = duk_get_string(ctx, 1);
  ActionId id =
      ActionManager::getActive()->assignCategory(actionName, categoryName);
  duk_push_uint(ctx, id);
  return 1;
}

// Actions.post
static duk_ret_t scriptPostAction(duk_context* ctx) {
  // Param extraction and type validation
  ActionId id = 0;
  if (duk_is_string(ctx, 0)) {
    const char* actionName = duk_get_string(ctx, 0);
    id = getId(actionName);
  } else if (duk_is_number(ctx, 0)) {
    id = duk_get_uint(ctx, 0);
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Actions.post: Invalid action identifier type");
    return 0;
  }
  if (!duk_is_array(ctx, 1)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Actions.post: Invalid data array");
    return 0;
  }

  // Stack top is a data array; wrap it in a Dictionary interface and enumerate
  PrimitiveVariant dataItem;
  Vector<PrimitiveVariant> actionData;
  Dictionary dictWrapper(ctx);
  for (Uint32 index = 0; index < dictWrapper.end(); ++index) {
    if (!dictWrapper.getVariantAt(index, dataItem)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "[Script] Actions.post: Invalid data item at index %u",
                   index);
      return 0;
    }
    actionData.push_back(dataItem);
  }
  ActionManager::getActive()->post({id, actionData});
  return 0;
}

// Actions.subscribe
static duk_ret_t scriptSubscribe(duk_context* ctx) {
  if (!duk_is_string(ctx, 0) || !duk_is_function(ctx, 1)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Actions.subscribe: Invalid parameter type(s)");
    return 0;
  }

  // TODO: Find a better way to not duplicate subscriptions on reload
  if (currentScriptContext && !currentScriptContext->initialized()) {
    // Register the handler for this action
    const char* catName = duk_get_string(ctx, 0);
    static ScriptContext* prevActiveContext = nullptr;
    static ActionHandlerPtr handler;
    if (prevActiveContext != currentScriptContext) {
    }
    // subscribe needs to deduplicate per category using pointer comparison
    // TODO: ScriptContexts can also be DESTROYED on resource change; need
    // UNsubscribe
    ActionCategoryId catId =
        ActionManager::getActive()->subscribe(handler, catName);
  }

  // Store a callback function reference in the stash
  duk_push_heap_stash(ctx);
  duk_get_prop_string(ctx, -1, "categoryHandlers");
  duk_dup(ctx, 1);
  duk_put_prop_index(ctx, -2, catId);
  return 0;
}

// Helpers.getId
static duk_ret_t scriptGetId(duk_context* ctx) {
  if (duk_is_string(ctx, 0)) {
    const char* key = duk_get_string(ctx, 0);
    Id keyId = getId(key);
    duk_push_number(ctx, (duk_double_t)keyId);
  } else {
    duk_push_number(ctx, 0.0);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[Script] Helpers.getId: Param 0 is not a string");
  }
  return 1;
}
/***** End of script helper functions *****/

struct ScriptContext::Impl : public Dictionary {
  explicit Impl() : Dictionary(), initialized(false) {
    setupGlobalEnv();

    // Set up an action callback array in the heap stash for helpers to use;
    // only needs to happen once after Dict heap creation, not on every load.
    duk_require_stack(ctx, 2);
    duk_push_heap_stash(ctx);
    duk_push_bare_array(ctx);
    duk_put_prop_literal(ctx, -2, "categoryHandlers");
    duk_pop(ctx);
  }

  ~Impl() {}

  // Interface class needs access to load(), full select(), etc.
  friend class ScriptContext;

  void registerFunc(const String& path, duk_c_function func, duk_idx_t nargs) {
    duk_require_stack(ctx, 1);
    duk_push_global_object(ctx);
    size_t depth = 1 + select(path.c_str(), true, false);
    duk_require_stack(ctx, 1);
    duk_idx_t length = nargs == DUK_VARARGS ? 1 : nargs;
    duk_push_c_lightfunc(ctx, func, nargs, length, 0);
    duk_put_prop(ctx, -3);
    unwind(depth);
  }

  void setupGlobalEnv() {
    // Get the automatically-managed heap/context from the inherited Dictionary
    ctx = getContext();

    // Register a debug logger and module loader
    registerFunc("console.log", &scriptConsoleLog, 1);
    registerFunc("require", &scriptRequire, 1);

    // Register Actions and Helpers bindings
    registerFunc("Actions.assignCategory", &scriptAssignCategory, 2);
    registerFunc("Actions.post", &scriptPostAction, 2);
    registerFunc("Actions.subscribe", &scriptSubscribe, 2);
    registerFunc("Helpers.getId", &scriptGetId, 1);

    // Create a globalThis reference to the global object
    duk_push_global_object(ctx);
    duk_push_global_object(ctx);
    duk_put_prop_string(ctx, -2, "globalThis");
    duk_pop(ctx);
  }

  duk_context* ctx;
  bool initialized;
};

RENITY_API ScriptContext::ScriptContext() {
  pimpl_ = new Impl();
  currentScriptContext = this;
  pimpl_->initialized = evalFile("/assets/scripts/init.js");
}

RENITY_API ScriptContext::~ScriptContext() {
  if (currentScriptContext == this) currentScriptContext = nullptr;
  delete pimpl_;
}
/*
// Error/debug logger helper for bound functions
static void loggerHelper(duk_context* ctx, const char* func, const char* msg) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "[Script] console.log: %s",
duk_safe_to_string(ctx, 0)); return 0;
}
*/
RENITY_API bool ScriptContext::initialized() { return pimpl_->initialized; }

RENITY_API bool ScriptContext::evalFile(String path) {
  StringBufferPtr buf =
      ResourceManager::getActive()->get<StringBuffer>(path.c_str());
  if (!buf->length()) return false;
  duk_int_t result =
      duk_peval_lstring(pimpl_->ctx, buf->getCStr(), buf->length());
  bool success = true;
  if (result == 0) {
    if (duk_get_boolean_default(pimpl_->ctx, -1, 1) == 0) {
      success = false;
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "ScriptContext::evalFile: '%s' compiled successfully "
                   "but returned a boolean false.",
                   path.c_str());
      SDL_SetError("Script evaluation returned false.");
    }
  } else {
    success = false;
    const char* error = duk_safe_to_string(pimpl_->ctx, -1);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "ScriptContext::evalFile: '%s' failed with '%s'", path.c_str(),
                 error);
    SDL_SetError("Script evaluation encountered '%s'.", error);
  }
  duk_pop(pimpl_->ctx);
  return success;
}

RENITY_API void ScriptContext::registerFunc(String path, duk_c_function func,
                                            duk_idx_t nargs) {
  pimpl_->registerFunc(path, func, nargs);
}

RENITY_API void ScriptContext::load(SDL_RWops* src) {
  pimpl_->load(src);

  // Reset env and rerun init script to set things up on JS side
  pimpl_->setupGlobalEnv();
  pimpl_->initialized = evalFile("/assets/scripts/init.js");
}

RENITY_API void ScriptContext::handleAction(const ActionCategoryId categoryId,
                                            const Action* action) {
  duk_idx_t origTop = duk_get_top(pimpl_->ctx);
  duk_push_heap_stash(pimpl_->ctx);
  if (!duk_get_prop_string(pimpl_->ctx, -1, "categoryHandlers") ||
      !duk_get_prop_index(pimpl_->ctx, -1, categoryId) ||
      !duk_is_function(pimpl_->ctx, -1)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "ScriptActionHandler::handleAction: JS categoryHandler is missing "
        "for "
        "ActionCategoryId 0x%#08x ('%s')",
        categoryId,
        ActionManager::getActive()->getNameFromId(categoryId).c_str());
    duk_set_top(pimpl_->ctx, origTop);
    return;
  }

  // Use the action name and an array of values as args for the actionHandler
  duk_push_string(pimpl_->ctx, action->getName().c_str());
  duk_push_bare_array(pimpl_->ctx);
  Dictionary dictWrapper(pimpl_->ctx);
  for (Uint32 dataIndex = 0; dataIndex < action->getDataCount(); ++dataIndex) {
    dictWrapper.putVariantAt(dataIndex, action->getData(dataIndex));
  }
  if (duk_pcall(pimpl_->ctx, 2) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "ScriptContext::handleAction: JS actionHandler failed for "
                 "ActionId 0x%#08x ('%s') with '%s'",
                 action->getId(), action->getName().c_str(),
                 duk_safe_to_string(pimpl_->ctx, -1));
  }
  // Ignore the return value until request/response actions are implemented
  duk_set_top(pimpl_->ctx, origTop);
}
}  // namespace renity
