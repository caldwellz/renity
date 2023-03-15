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
    depth = 0;
  }

  ~Impl() {
    duk_pop(ctx);
    duk_destroy_heap(ctx);
  }

  duk_context *ctx;
  Uint8 depth;
};

RENITY_API Dictionary::Dictionary() { pimpl_ = new Impl(); }

RENITY_API Dictionary::~Dictionary() { delete this->pimpl_; }

RENITY_API void Dictionary::load(SDL_RWops *src) {
  // Remove (pop) the current object, whether a default one or a loaded one
  duk_pop(pimpl_->ctx);

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
                 "attempting JSON decode.\n");
    try {
      duk_pop(pimpl_->ctx);
      duk_push_lstring(pimpl_->ctx, (const char *)buf, bufSize);
      duk_json_decode(pimpl_->ctx, -1);
    } catch (const std::runtime_error &e) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Dictionary::load: Failed to decode RWops as JSON or CBOR - "
                   "using empty object.\n");
      duk_pop(pimpl_->ctx);
      duk_push_bare_object(pimpl_->ctx);
    }
  }
  SDL_free(buf);

  if (duk_is_array(pimpl_->ctx, -1) || !duk_is_object(pimpl_->ctx, -1)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Dictionary::load: Decoded file is not an object - replacing "
                "with empty object.\n");
    duk_pop(pimpl_->ctx);
    duk_push_bare_object(pimpl_->ctx);
  }
}

RENITY_API bool Dictionary::saveJSON(const char *destPath) {
  // Encoders replace the underlying object in-place with the encoded value,
  // so we have to duplicate it, encode it, and then discard the encoding.
  duk_dup_top(pimpl_->ctx);
  const char *buf = duk_json_encode(pimpl_->ctx, -1);
  Uint32 bufLen = SDL_strlen(buf);
  bool success =
      RENITY_WriteBufferToPath(destPath, (const Uint8 *)buf, bufLen) == bufLen;
  duk_pop(pimpl_->ctx);
  return success;
}

RENITY_API bool Dictionary::saveCBOR(const char *destPath) {
  // Encoders replace the underlying object in-place with the encoded value,
  // so we have to duplicate it, encode it, and then discard the encoding.
  duk_dup_top(pimpl_->ctx);
  duk_cbor_encode(pimpl_->ctx, -1, 0);
  duk_size_t bufLen;
  const Uint8 *buf =
      (const Uint8 *)duk_get_buffer_data(pimpl_->ctx, -1, &bufLen);
  bool success =
      (duk_size_t)RENITY_WriteBufferToPath(destPath, buf, bufLen) == bufLen;
  duk_pop(pimpl_->ctx);
  return success;
}

bool Dictionary::traverseObj(const char *rawKey, bool autoCreate) {
  const char separator = '.';
  String key(rawKey);
  String token(rawKey);
  size_t pos = key.find(separator), lastPos = 0;
  while (pos != String::npos && pimpl_->depth < DUK_API_ENTRY_STACK) {
    token = key.substr(lastPos, pos - lastPos);
    if (token.length() == 0) {
      SDL_LogWarn(
          SDL_LOG_CATEGORY_APPLICATION,
          "Dictionary::traverseObj: Invalid subkey of '%s' at index %i.\n",
          rawKey, pos);
      return false;
    }
    duk_get_prop_string(pimpl_->ctx, -1, token.c_str());
    ++pimpl_->depth;
    if (!duk_is_object(pimpl_->ctx, -1)) {
      if (autoCreate) {
        duk_pop(pimpl_->ctx);
        duk_push_bare_object(pimpl_->ctx);
        duk_put_prop_string(pimpl_->ctx, -2, token.c_str());
        duk_get_prop_string(pimpl_->ctx, -1, token.c_str());
      } else {
        return false;
      }
    }
    lastPos = pos + 1;
    pos = key.find(separator, lastPos);
    token = key.substr(lastPos);
  }
  ++pimpl_->depth;
  duk_push_string(pimpl_->ctx, token.c_str());

  return true;
}

void Dictionary::unwindObj() {
  if (duk_is_object(pimpl_->ctx, -1)) {
    --pimpl_->depth;
  }
  for (; pimpl_->depth > 0; --pimpl_->depth) {
    duk_pop(pimpl_->ctx);
  }
}

#define DICT_IMPL_BASE(T, dukT, printSpec, printAdd)                          \
  template <>                                                                 \
  RENITY_API bool Dictionary::get<T>(const char *key, T *valOut) {            \
    if (!traverseObj(key, false) || !duk_get_prop(pimpl_->ctx, -2),           \
        !duk_is_##dukT(pimpl_->ctx, -1)) {                                    \
      SDL_LogDebug(                                                           \
          SDL_LOG_CATEGORY_APPLICATION,                                       \
          "Dictionary::get: Key or correct-type value not found for '%s'\n",  \
          key);                                                               \
      unwindObj();                                                            \
      return false;                                                           \
    }                                                                         \
    if (valOut) {                                                             \
      *valOut = (T)duk_get_##dukT(pimpl_->ctx, -1);                           \
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                              \
                   "Dictionary::get: '%s': " printSpec "\n", key,             \
                   *valOut printAdd);                                         \
    }                                                                         \
    unwindObj();                                                              \
    return true;                                                              \
  }                                                                           \
                                                                              \
  template <>                                                                 \
  RENITY_API bool Dictionary::put<T>(const char *key, T val) {                \
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,                                \
                 "Dictionary::put: '%s'=" printSpec "\n", key, val printAdd); \
    if (!traverseObj(key, true)) {                                            \
      unwindObj();                                                            \
      return false;                                                           \
    }                                                                         \
    duk_push_##dukT(pimpl_->ctx, val);                                        \
    duk_bool_t success = duk_put_prop(pimpl_->ctx, -3);                       \
    unwindObj();                                                              \
    return !!success;                                                         \
  }
#define DICT_IMPL(T, dukT, printSpec) DICT_IMPL_BASE(T, dukT, printSpec, )

// Supported dictionary/duktape types
DICT_IMPL_BASE(bool, boolean, "%s", ? "true" : "false")
DICT_IMPL(const char *, string, "%s")
DICT_IMPL(Uint8, number, "%u")
DICT_IMPL(Uint16, number, "%u")
DICT_IMPL(Uint32, number, "%u")
DICT_IMPL(Sint8, number, "%i")
DICT_IMPL(Sint16, number, "%i")
DICT_IMPL(Sint32, number, "%i")
DICT_IMPL(float, number, "%f")
DICT_IMPL(double, number, "%f")
}  // namespace renity
