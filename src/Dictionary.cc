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

namespace renity {
struct Dictionary::Impl {
  Impl() {
    ctx = duk_create_heap_default();
    duk_push_bare_object(ctx);
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
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Dictionary::load: Collapsed stack of %lu down to %lu.\n",
               origTop, duk_get_top(pimpl_->ctx));

  Uint8 *buf;
  Sint64 bufSize = RENITY_ReadBuffer(src, &buf);
  if (bufSize < 1) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::load: Invalid RWops (%li).\n", bufSize);
    duk_push_bare_object(pimpl_->ctx);
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
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::load: Failed to decode RWops as CBOR - "
                 "attempting JSON decode at stack=%lu.\n",
                 duk_get_top(pimpl_->ctx));
    try {
      duk_set_top(pimpl_->ctx, 0);
      duk_require_stack(pimpl_->ctx, 1);
      duk_push_lstring(pimpl_->ctx, (const char *)buf, bufSize);
      duk_json_decode(pimpl_->ctx, -1);
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
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
}

RENITY_API bool Dictionary::saveJSON(const char *destPath) {
  // Encoders replace the stack top in-place with the encoded value,
  // so we have to duplicate the root, encode it, and then discard the encoding.
  duk_dup(pimpl_->ctx, 0);
  const char *buf = duk_json_encode(pimpl_->ctx, -1);
  Uint32 bufLen = SDL_strlen(buf);
  bool success =
      RENITY_WriteBufferToPath(destPath, (const Uint8 *)buf, bufLen) == bufLen;
  duk_pop(pimpl_->ctx);
  return success;
}

RENITY_API bool Dictionary::saveCBOR(const char *destPath) {
  // Encoders replace the stack top in-place with the encoded value,
  // so we have to duplicate the obj, encode it, and then discard the encoding.
  duk_dup(pimpl_->ctx, 0);
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
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Dictionary::select: %lu deep before selecting '%s'.\n",
               duk_get_top(pimpl_->ctx) - 1, path);

  // Short-circuit if an edge value is already selected -
  // caller needs to unwind or replace it with an object first
  if (!duk_is_object(pimpl_->ctx, -1)) return 0;

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
        SDL_LogDebug(
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
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
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

  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Dictionary::select: '%s' -> %u deep.\n", path, depth + 1);
  return ++depth;
}

RENITY_API size_t Dictionary::selectIndex(Uint32 index, bool autoCreate) {
  SDL_LogDebug(
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
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Dictionary::unwind: %u - %u\n",
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
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Dictionary::putArray: '%s' is already an Array\n", key);
    unwind(depth + 1);
    return true;
  }

  // Replace whatever value is (or isn't) there with an array
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Dictionary::putArray: '%s'=[]\n",
               key);
  duk_pop(pimpl_->ctx);
  duk_push_bare_array(pimpl_->ctx);
  duk_bool_t success = duk_put_prop(pimpl_->ctx, -3);
  unwind(depth - 1);
  return !!success;
}
/*
RENITY_API bool Dictionary::putObject(const char *key) {
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Dictionary::putObject: '%s'={}\n",
               key);
  size_t depth = select(key, true, true);
  // TODO: If the edge is not an object, convert it to one
  unwind(depth);
  return !!depth;
}
*/
#define DICT_IMPL_BASE(T, dukExt, dukInt, printSpec, printAdd)               \
  template <>                                                                \
  RENITY_API bool Dictionary::get<T>(const char *key, T *valOut) {           \
    size_t depth = select(key, false, true);                                 \
    if (!depth || !duk_is_##dukInt(pimpl_->ctx, -1)) {                       \
      SDL_LogDebug(                                                          \
          SDL_LOG_CATEGORY_APPLICATION,                                      \
          "Dictionary::get: Key or correct-type value not found for '%s'\n", \
          key);                                                              \
      unwind(depth);                                                         \
      return false;                                                          \
    }                                                                        \
    if (valOut) {                                                            \
      *valOut = (T)duk_get_##dukExt(pimpl_->ctx, -1);                        \
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                             \
                   "Dictionary::get: '%s': " printSpec "\n", key,            \
                   *valOut printAdd);                                        \
    }                                                                        \
    unwind(depth);                                                           \
    return true;                                                             \
  }                                                                          \
                                                                             \
  template <>                                                                \
  RENITY_API bool Dictionary::getIndex<T>(Uint32 index, T * valOut) {        \
    size_t depth = selectIndex(index, false);                                \
    if (!depth || !duk_is_##dukInt(pimpl_->ctx, -1)) {                       \
      SDL_LogDebug(                                                          \
          SDL_LOG_CATEGORY_APPLICATION,                                      \
          "Dictionary::get: Correct-type value not found at index %u\n",     \
          index);                                                            \
      unwind(depth);                                                         \
      return false;                                                          \
    }                                                                        \
    if (valOut) {                                                            \
      *valOut = (T)duk_get_##dukExt(pimpl_->ctx, -1);                        \
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                             \
                   "Dictionary::get: [%u]: " printSpec "\n", index,          \
                   *valOut printAdd);                                        \
    }                                                                        \
    unwind(depth);                                                           \
    return true;                                                             \
  }                                                                          \
                                                                             \
  template <>                                                                \
  RENITY_API bool Dictionary::push<T>(T val) {                               \
    if (!duk_is_object(pimpl_->ctx, -1)) {                                   \
      SDL_LogError(                                                          \
          SDL_LOG_CATEGORY_APPLICATION,                                      \
          "Dictionary::push: Selected edge is not an object or array");      \
      return false;                                                          \
    }                                                                        \
    duk_size_t index = duk_get_length(pimpl_->ctx, -1);                      \
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                               \
                 "Dictionary::push: [%lu]=" printSpec "\n", index,           \
                 val printAdd);                                              \
    duk_require_stack(pimpl_->ctx, 1);                                       \
    duk_push_##dukExt(pimpl_->ctx, val);                                     \
    return !!duk_put_prop_index(pimpl_->ctx, -2, index);                     \
  }                                                                          \
                                                                             \
  template <>                                                                \
  RENITY_API bool Dictionary::put<T>(const char *key, T val) {               \
    size_t depth = select(key, true, false);                                 \
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                               \
                 "Dictionary::put: '%s'(%u)=" printSpec "\n", key, depth,    \
                 val printAdd);                                              \
    if (!depth) {                                                            \
      return false;                                                          \
    }                                                                        \
    duk_require_stack(pimpl_->ctx, 1);                                       \
    duk_push_##dukExt(pimpl_->ctx, val);                                     \
    duk_bool_t success = duk_put_prop(pimpl_->ctx, -3);                      \
    unwind(depth - 1);                                                       \
    return !!success;                                                        \
  }                                                                          \
                                                                             \
  template <>                                                                \
  RENITY_API bool Dictionary::putIndex<T>(Uint32 index, T val) {             \
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                               \
                 "Dictionary::put: [%u]=" printSpec "\n", index,             \
                 val printAdd);                                              \
    if (!duk_is_object(pimpl_->ctx, -1)) {                                   \
      SDL_LogError(                                                          \
          SDL_LOG_CATEGORY_APPLICATION,                                      \
          "Dictionary::put: Selected edge is not an object or array");       \
      return false;                                                          \
    }                                                                        \
    duk_require_stack(pimpl_->ctx, 1);                                       \
    duk_push_##dukExt(pimpl_->ctx, val);                                     \
    return !!duk_put_prop_index(pimpl_->ctx, -2, index);                     \
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
