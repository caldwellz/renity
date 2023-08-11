/****************************************************
 * Dictionary.cc: Configuration resource class      *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Dictionary.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "3rdparty/duktape/duktape.h"
#include "utils/rwops_utils.h"
#include "utils/string_helpers.h"

namespace renity {
struct Dictionary::Impl {
  Impl() {
    ctx = duk_create_heap_default();
    duk_push_bare_object(ctx);
    duk_set_global_object(ctx);
    duk_push_global_object(ctx);
  }

  ~Impl() {
    duk_set_top(ctx, 0);
    duk_destroy_heap(ctx);
  }

  duk_context *ctx;
};

RENITY_API Dictionary::Dictionary() { pimpl_ = new Impl(); }

RENITY_API Dictionary::~Dictionary() { delete this->pimpl_; }

RENITY_API void Dictionary::load(SDL_RWops *src) {
  // Delete the current object (whether a default one or a loaded one) by
  // clearing the stack
  duk_idx_t origTop = duk_get_top(pimpl_->ctx);
  duk_set_top(pimpl_->ctx, 0);
  duk_require_stack(pimpl_->ctx, 1);
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::load: Collapsed stack of %lu down to %lu.\n",
                 origTop, duk_get_top(pimpl_->ctx));

  Uint8 *buf;
  Sint64 bufSize = RENITY_ReadBuffer(src, &buf);
  if (bufSize < 1) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::load: Invalid RWops (%li).\n", bufSize);
    duk_push_bare_object(pimpl_->ctx);
    duk_set_global_object(pimpl_->ctx);
    duk_push_global_object(pimpl_->ctx);
    return;
  }

  duk_push_external_buffer(pimpl_->ctx);
  duk_config_buffer(pimpl_->ctx, -1, buf, bufSize);
  try {
    duk_cbor_decode(pimpl_->ctx, -1, 0);
  } catch (const std::runtime_error &e) {
    // Yes, we're continuing after an otherwise "fatal" Duktape error...
    // However, it should be safe since we're doing simple operations, not
    // executing Javascript, and know exactly what the context stack looks like.
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::load: Failed to decode RWops as CBOR - "
                   "attempting JSON decode at stack=%lu.\n",
                   duk_get_top(pimpl_->ctx));
    try {
      duk_set_top(pimpl_->ctx, 0);
      duk_require_stack(pimpl_->ctx, 1);
      duk_push_lstring(pimpl_->ctx, (const char *)buf, bufSize);
      duk_json_decode(pimpl_->ctx, -1);
      SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                     "Dictionary::load: Decoded RWops as JSON at stack=%lu.\n",
                     duk_get_top(pimpl_->ctx));
    } catch (const std::runtime_error &e) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::load: Failed to decode RWops as JSON or CBOR - "
                   "falling through at stack=%lu.\n",
                   duk_get_top(pimpl_->ctx));
    }
  }
  SDL_free(buf);

  if (duk_is_array(pimpl_->ctx, -1) || !duk_is_object(pimpl_->ctx, -1)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "Dictionary::load: Decoded file is an array or not an object - "
        "replacing with empty object.\n");
    duk_set_top(pimpl_->ctx, 0);
    duk_require_stack(pimpl_->ctx, 1);
    duk_push_bare_object(pimpl_->ctx);
  }

  duk_set_global_object(pimpl_->ctx);
  duk_push_global_object(pimpl_->ctx);
}

RENITY_API bool Dictionary::save(const char *destPath, bool selectionOnly) {
  if (!destPath) return false;
  if (endsWith(toLower(String(destPath)), String(".json"))) {
    return saveJSON(destPath, selectionOnly);
  }
  return saveCBOR(destPath, selectionOnly);
}

RENITY_API bool Dictionary::saveJSON(const char *destPath, bool selectionOnly) {
  // Encoders replace the stack top in-place with the encoded value, so we
  // have to duplicate something, encode it, and then discard the encoding.
  duk_require_stack(pimpl_->ctx, 1);
  if (selectionOnly) {
    if (!duk_is_object(pimpl_->ctx, -1)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::saveJSON: Tried to save a non-object/non-array "
                   "to '%s'.\n",
                   destPath);
      return false;
    }
    duk_dup_top(pimpl_->ctx);
  } else {
    duk_push_global_object(pimpl_->ctx);
  }
  const char *buf = duk_json_encode(pimpl_->ctx, -1);
  Uint32 bufLen = SDL_strlen(buf);
  bool success =
      RENITY_WriteBufferToPath(destPath, (const Uint8 *)buf, bufLen) == bufLen;
  duk_pop(pimpl_->ctx);
  return success;
}

RENITY_API bool Dictionary::saveCBOR(const char *destPath, bool selectionOnly) {
  // Encoders replace the stack top in-place with the encoded value, so we
  // have to duplicate something, encode it, and then discard the encoding.
  duk_require_stack(pimpl_->ctx, 1);
  if (selectionOnly) {
    if (!duk_is_object(pimpl_->ctx, -1)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::saveCBOR: Tried to save a non-object/non-array "
                   "to '%s'.\n",
                   destPath);
      return false;
    }
    duk_dup_top(pimpl_->ctx);
  } else {
    duk_push_global_object(pimpl_->ctx);
  }
  duk_cbor_encode(pimpl_->ctx, -1, 0);
  duk_size_t bufLen;
  const Uint8 *buf =
      (const Uint8 *)duk_get_buffer_data(pimpl_->ctx, -1, &bufLen);
  bool success =
      (duk_size_t)RENITY_WriteBufferToPath(destPath, buf, bufLen) == bufLen;
  duk_pop(pimpl_->ctx);
  return success;
}

RENITY_API size_t Dictionary::select(const char *path, bool autoCreate,
                                     bool loadValue) {
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::select: %lu deep before selecting '%s'.\n",
                 duk_get_top(pimpl_->ctx) - 1, path);

  // Short-circuit on invalid path or if an edge value is already selected -
  // caller needs to unwind or otherwise handle this themselves.
  if (!path || !duk_is_object(pimpl_->ctx, -1)) return 0;

  const char separator = '.';
  String key(path);
  String token(path);
  size_t pos = key.find(separator), lastPos = 0, depth = 0;
  while (pos != String::npos) {
    token = key.substr(lastPos, pos - lastPos);
    if (token.length() == 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::select: Invalid path '%s' at index %li.\n",
                   path, pos);
      unwind(depth);
      return 0;
    }
    duk_require_stack(pimpl_->ctx, 1);
    duk_get_prop_lstring(pimpl_->ctx, -1, token.c_str(), token.length());
    ++depth;
    if (!duk_is_object(pimpl_->ctx, -1)) {
      if (autoCreate) {
        duk_pop(pimpl_->ctx);
        duk_push_bare_object(pimpl_->ctx);
        duk_put_prop_lstring(pimpl_->ctx, -2, token.c_str(), token.length());
        duk_get_prop_lstring(pimpl_->ctx, -1, token.c_str(), token.length());
      } else {
        SDL_LogVerbose(
            SDL_LOG_CATEGORY_APPLICATION,
            "Dictionary::select: Not autocreating missing subkey %s of '%s'.\n",
            token.c_str(), path);
        unwind(depth);
        return 0;
      }
    }
    lastPos = pos + 1;
    pos = key.find(separator, lastPos);
  }

  token = key.substr(lastPos);
  duk_require_stack(pimpl_->ctx, 1);
  if (loadValue) {
    duk_get_prop_lstring(pimpl_->ctx, -1, token.c_str(), token.length());
    if (autoCreate && duk_is_undefined(pimpl_->ctx, -1)) {
      SDL_LogVerbose(
          SDL_LOG_CATEGORY_APPLICATION,
          "Dictionary::select: Autocreating edge subkey %s of '%s'.\n",
          token.c_str(), path);
      duk_pop(pimpl_->ctx);
      duk_push_bare_object(pimpl_->ctx);
      duk_put_prop_lstring(pimpl_->ctx, -2, token.c_str(), token.length());
      duk_get_prop_lstring(pimpl_->ctx, -1, token.c_str(), token.length());
    }
  } else {
    duk_push_lstring(pimpl_->ctx, token.c_str(), token.length());
  }

  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::select: '%s' -> %u deep.\n", path, depth + 1);
  return ++depth;
}

RENITY_API size_t Dictionary::selectIndex(Uint32 index, bool autoCreate) {
  SDL_LogVerbose(
      SDL_LOG_CATEGORY_APPLICATION,
      "Dictionary::select: %lu deep past obj before selecting idx %lu.\n",
      duk_get_top(pimpl_->ctx) - 1, index);

  // Short-circuit if an edge value is already selected -
  // caller needs to unwind or replace it with an object first
  if (!duk_is_object(pimpl_->ctx, -1)) return 0;

  duk_require_stack(pimpl_->ctx, 1);
  if (duk_get_prop_index(pimpl_->ctx, -1, index)) return 1;
  duk_pop(pimpl_->ctx);
  if (!autoCreate) return 0;
  duk_push_bare_object(pimpl_->ctx);
  duk_put_prop_index(pimpl_->ctx, -2, index);
  return duk_get_prop_index(pimpl_->ctx, -1, index);
}

RENITY_API void Dictionary::unwind(size_t depth) {
  // The base object should always be the first item on the stack
  const size_t maxDepth = duk_get_top(pimpl_->ctx) - 1;
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Dictionary::unwind: %u - %u\n",
                 maxDepth, depth);

  // Short-circuit for e.g. invalid selects
  if (depth == 0) return;

  if (depth >= maxDepth) {
    // Clear the stack down to the base object, unwinding all selects
    duk_set_top(pimpl_->ctx, 1);
  } else {
    duk_set_top(pimpl_->ctx, -depth);
  }
}

RENITY_API Uint32 Dictionary::begin(const char *key) {
  Uint32 index = 0;
  size_t depth = select(key, false, true);
  if (!duk_is_array(pimpl_->ctx, -1)) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::begin: Selection is not an Array\n");
    index = UINT32_MAX;
  }
  unwind(depth);
  return index;
}

RENITY_API Uint32 Dictionary::end(const char *key) {
  Uint32 index = UINT32_MAX;
  size_t depth = select(key, false, true);
  if (duk_is_array(pimpl_->ctx, -1)) {
    index = duk_get_length(pimpl_->ctx, -1);
  } else {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::end: Selection is not an Array\n");
  }
  unwind(depth);
  return index;
}

RENITY_API Uint32 Dictionary::enumerate(
    const char *path,
    const FuncPtr<bool(Dictionary &, const String &)> &callback) {
  Uint32 props = 0;
  const size_t selectDepth = select(path, false, true);
  if ((path && !selectDepth) || !duk_is_object(pimpl_->ctx, -1)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "Dictionary::enumerate: Could not enumerate a non-object in '%s'.\n",
        path ? path : "(current selection)");
    unwind(selectDepth);
    return 0;
  }

  duk_require_stack(pimpl_->ctx, 3);
  duk_enum(pimpl_->ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY);

  // Move the current selection in front of the enumerator so the cb can use it
  duk_pull(pimpl_->ctx, -2);

  // Save initial depth to check/unwind the stack after each callback
  const size_t enumDepth = duk_get_top(pimpl_->ctx);

  bool keepGoing = true;
  while (keepGoing && duk_next(pimpl_->ctx, -2, 0)) {
    // Copy the key and replace it with the value, effectively doing a select()
    String key(duk_get_string(pimpl_->ctx, -1));
    duk_get_prop(pimpl_->ctx, -2);

    // TODO: Make the JSON encoder leave empty array slots empty, NOT null
    if (duk_is_null_or_undefined(pimpl_->ctx, -1)) {
      duk_pop(pimpl_->ctx);
      continue;
    }
    keepGoing = callback(*this, key);
    ++props;

    // Bail out if the callback was naughty and unwound past its starting point
    const size_t currentTop = duk_get_top(pimpl_->ctx);
    if (currentTop < enumDepth) {
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                  "Dictionary::enumerate: Callback unwound past :%lu to :%lu - "
                  "cancelling enumeration.\n",
                  enumDepth - 1, currentTop - 1);

      // Unwind the enumeration and initial select() as needed
      const size_t origTop = enumDepth - selectDepth - 1;
      if (currentTop > origTop) {
        duk_set_top(pimpl_->ctx, origTop);
      }
      return props;
    }

    // Unwind to the path selection, cleaning up whatever was added to the stack
    duk_set_top(pimpl_->ctx, enumDepth);
  }

  // Remove the enumerator, leaving the existing selection on top if used
  if (selectDepth) {
    unwind(selectDepth + 1);
  } else {
    duk_remove(pimpl_->ctx, -2);
  }

  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::enumerate: Finished after %lu props with %li -> "
                 "%li stack change.\n",
                 props, (Sint32)enumDepth - selectDepth - 2,
                 (Sint32)duk_get_top(pimpl_->ctx) - 1);
  return props;
}

RENITY_API Uint32 Dictionary::enumerateArray(
    const char *path,
    const FuncPtr<bool(Dictionary &, const Uint32 &)> &callback) {
  Uint32 props = 0;
  const size_t selectDepth = select(path, false, true);
  if ((path && !selectDepth) || !duk_is_object(pimpl_->ctx, -1)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "Dictionary::enumerateArray: Could not enumerate a non-indexable "
        "in '%s'.\n",
        path ? path : "(current selection)");
    unwind(selectDepth);
    return 0;
  }

  duk_require_stack(pimpl_->ctx, 3);
  duk_enum(pimpl_->ctx, -1,
           DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_ARRAY_INDICES_ONLY);

  // Move the current selection in front of the enumerator so the cb can use it
  duk_pull(pimpl_->ctx, -2);

  // Save initial depth to check/unwind the stack after each callback
  const size_t enumDepth = duk_get_top(pimpl_->ctx);

  bool keepGoing = true;
  while (keepGoing && duk_next(pimpl_->ctx, -2, 0)) {
    // Copy the key and replace it with the value, effectively doing a select()
    Uint32 key = duk_to_uint(pimpl_->ctx, -1);
    duk_get_prop(pimpl_->ctx, -2);

    // TODO: Make the JSON encoder leave empty array slots empty, NOT null
    if (duk_is_null_or_undefined(pimpl_->ctx, -1)) {
      duk_pop(pimpl_->ctx);
      continue;
    }
    keepGoing = callback(*this, key);
    ++props;

    // Bail out if the callback was naughty and unwound past its starting point
    const size_t currentTop = duk_get_top(pimpl_->ctx);
    if (currentTop < enumDepth) {
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                  "Dictionary::enumerateArray: Callback unwound past :%lu to "
                  ":%lu - cancelling enumeration.\n",
                  enumDepth - 1, currentTop - 1);

      // Unwind the enumeration and initial select() as needed
      const size_t origTop = enumDepth - selectDepth - 1;
      if (currentTop > origTop) {
        duk_set_top(pimpl_->ctx, origTop);
      }
      return props;
    }

    // Unwind to the path selection, cleaning up whatever was added to the stack
    duk_set_top(pimpl_->ctx, enumDepth);
  }

  // Remove the enumerator, leaving the existing selection on top if used
  if (selectDepth) {
    unwind(selectDepth + 1);
  } else {
    duk_remove(pimpl_->ctx, -2);
  }

  SDL_LogVerbose(
      SDL_LOG_CATEGORY_APPLICATION,
      "Dictionary::enumerateArray: Finished after %lu props with %li "
      "-> %li stack change.\n",
      props, (Sint32)enumDepth - selectDepth - 2,
      (Sint32)duk_get_top(pimpl_->ctx) - 1);
  return props;
}

RENITY_API bool Dictionary::putArray(const char *key) {
  size_t depth = select(key, true, false);
  if (!depth) {
    return false;
  }
  duk_require_stack(pimpl_->ctx, 2);
  duk_dup_top(pimpl_->ctx);
  duk_get_prop(pimpl_->ctx, -3);

  // If the key is already an array, we're done
  if (duk_is_array(pimpl_->ctx, -1)) {
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::putArray: '%s' is already an Array\n", key);
    unwind(depth + 1);
    return true;
  }

  // Replace whatever value is (or isn't) there with an array
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::putArray: '%s'=[]\n", key);
  duk_pop(pimpl_->ctx);
  duk_push_bare_array(pimpl_->ctx);
  duk_bool_t success = duk_put_prop(pimpl_->ctx, -3);
  unwind(depth - 1);
  return !!success;
}
/*
RENITY_API bool Dictionary::putObject(const char *key) {
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Dictionary::putObject:
'%s'={}\n", key); size_t depth = select(key, true, true);
  // TODO: If the edge is not an object, convert it to one
  unwind(depth);
  return !!depth;
}
*/

RENITY_API void *Dictionary::getContext() { return pimpl_->ctx; }

#define DICT_IMPL_BASE(T, dukExt, dukInt, printSpec, printAdd)                \
  template <>                                                                 \
  RENITY_API bool Dictionary::get<T>(const char *key, T *valOut) {            \
    size_t depth = select(key, false, true);                                  \
    if (!duk_is_##dukInt(pimpl_->ctx, -1)) {                                  \
      SDL_LogDebug(                                                           \
          SDL_LOG_CATEGORY_APPLICATION,                                       \
          "Dictionary::get: Key or correct-type value not found for '%s'\n",  \
          key);                                                               \
      unwind(depth);                                                          \
      return false;                                                           \
    }                                                                         \
    if (valOut) {                                                             \
      *valOut = (T)duk_get_##dukExt(pimpl_->ctx, -1);                         \
      SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,                            \
                     "Dictionary::get: '%s': " printSpec "\n", key,           \
                     *valOut printAdd);                                       \
    }                                                                         \
    unwind(depth);                                                            \
    return true;                                                              \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  RENITY_API bool Dictionary::getIndex<T>(Uint32 index, T * valOut) {         \
    size_t depth = selectIndex(index, false);                                 \
    if (!depth || !duk_is_##dukInt(pimpl_->ctx, -1)) {                        \
      SDL_LogDebug(                                                           \
          SDL_LOG_CATEGORY_APPLICATION,                                       \
          "Dictionary::get: Correct-type value not found at index %u\n",      \
          index);                                                             \
      unwind(depth);                                                          \
      return false;                                                           \
    }                                                                         \
    if (valOut) {                                                             \
      *valOut = (T)duk_get_##dukExt(pimpl_->ctx, -1);                         \
      SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,                            \
                     "Dictionary::get: [%u]: " printSpec "\n", index,         \
                     *valOut printAdd);                                       \
    }                                                                         \
    unwind(depth);                                                            \
    return true;                                                              \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  RENITY_API bool Dictionary::push<T>(T val) {                                \
    if (!duk_is_object(pimpl_->ctx, -1)) {                                    \
      SDL_LogError(                                                           \
          SDL_LOG_CATEGORY_APPLICATION,                                       \
          "Dictionary::push: Selected edge is not an object or array");       \
      return false;                                                           \
    }                                                                         \
    duk_size_t index = duk_get_length(pimpl_->ctx, -1);                       \
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,                              \
                   "Dictionary::push: [%lu]=" printSpec "\n", index,          \
                   val printAdd);                                             \
    duk_require_stack(pimpl_->ctx, 1);                                        \
    duk_push_##dukExt(pimpl_->ctx, val);                                      \
    return !!duk_put_prop_index(pimpl_->ctx, -2, index);                      \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  RENITY_API bool Dictionary::put<T>(const char *key, T val) {                \
    size_t depth = select(key, true, false);                                  \
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,                              \
                   "Dictionary::put: '%s' (%lu)=" printSpec "\n", key, depth, \
                   val printAdd);                                             \
    if (!depth) {                                                             \
      return false;                                                           \
    }                                                                         \
    duk_require_stack(pimpl_->ctx, 1);                                        \
    duk_push_##dukExt(pimpl_->ctx, val);                                      \
    duk_bool_t success = duk_put_prop(pimpl_->ctx, -3);                       \
    unwind(depth - 1);                                                        \
    return !!success;                                                         \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  RENITY_API bool Dictionary::putIndex<T>(Uint32 index, T val) {              \
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,                              \
                   "Dictionary::put: [%u]=" printSpec "\n", index,            \
                   val printAdd);                                             \
    if (!duk_is_object(pimpl_->ctx, -1)) {                                    \
      SDL_LogError(                                                           \
          SDL_LOG_CATEGORY_APPLICATION,                                       \
          "Dictionary::put: Selected edge is not an object or array");        \
      return false;                                                           \
    }                                                                         \
    duk_require_stack(pimpl_->ctx, 1);                                        \
    duk_push_##dukExt(pimpl_->ctx, val);                                      \
    return !!duk_put_prop_index(pimpl_->ctx, -2, index);                      \
  }
#define DICT_IMPL(T, dukExt, dukInt, printSpec) \
  DICT_IMPL_BASE(T, dukExt, dukInt, printSpec, )

// Supported dictionary/duktape types
DICT_IMPL_BASE(bool, boolean, boolean, "%s", ? "true" : "false")
DICT_IMPL(const char *, string, string, "%s")
DICT_IMPL(Uint8, uint, number, "%u")
DICT_IMPL(Uint16, uint, number, "%u")
DICT_IMPL(Uint32, uint, number, "%u")
DICT_IMPL(Sint8, int, number, "%i")
DICT_IMPL(Sint16, int, number, "%i")
DICT_IMPL(Sint32, int, number, "%i")
DICT_IMPL(float, number, number, "%f")
DICT_IMPL(double, number, number, "%f")
}  // namespace renity
