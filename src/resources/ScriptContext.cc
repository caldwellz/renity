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

#include "ResourceManager.h"
#include "resources/StringBuffer.h"

namespace renity {
struct ScriptContext::Impl {
  explicit Impl() : initialized(false) {}
  ~Impl() {}

  duk_context* ctx;
  bool initialized;
};

RENITY_API ScriptContext::ScriptContext() : Dictionary() {
  pimpl_ = new Impl();
  setupGlobalEnv();
}

RENITY_API ScriptContext::~ScriptContext() { delete pimpl_; }
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
  duk_require_stack(pimpl_->ctx, 1);
  duk_push_global_object(pimpl_->ctx);
  size_t depth = 1 + select(path.c_str(), true, false);
  duk_require_stack(pimpl_->ctx, 1);
  duk_push_c_lightfunc(pimpl_->ctx, func, nargs, nargs, 0);
  duk_put_prop(pimpl_->ctx, -3);
  unwind(depth);
}

RENITY_API void ScriptContext::load(SDL_RWops* src) {
  // Dictionary resets the global object on reload, so recreate the environment
  Dictionary::load(src);
  setupGlobalEnv();
}

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

RENITY_API void ScriptContext::setupGlobalEnv() {
  // Get the automatically-managed heap/context from the inherited Dictionary
  pimpl_->ctx = this->getContext();

  // Register a debug logger and module loader
  registerFunc("console.log", &scriptConsoleLog, 1);
  registerFunc("require", &scriptRequire, 1);

  // Create a globalThis reference to the global object
  duk_push_global_object(pimpl_->ctx);
  duk_push_global_object(pimpl_->ctx);
  duk_put_prop_string(pimpl_->ctx, -2, "globalThis");
  duk_push_string(pimpl_->ctx, "bar");
  duk_put_prop_string(pimpl_->ctx, -2, "foo");
  duk_pop(pimpl_->ctx);

  // (Re)run init script to set things up on JS side
  pimpl_->initialized = evalFile("/assets/scripts/init.js");
}
}  // namespace renity
