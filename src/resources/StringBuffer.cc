/****************************************************
 * StringBuffer.cc: Fully-buffered String resource  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/StringBuffer.h"

#include <SDL3/SDL_log.h>

#include "utils/rwops_utils.h"

namespace renity {
struct StringBuffer::Impl {
  explicit Impl() {}

  String content;
};

RENITY_API StringBuffer::StringBuffer() { pimpl_ = new Impl(); }

RENITY_API StringBuffer::~StringBuffer() { delete pimpl_; }

RENITY_API String StringBuffer::getString() const { return pimpl_->content; }

RENITY_API const char *StringBuffer::getCStr() const {
  return pimpl_->content.c_str();
}

RENITY_API size_t StringBuffer::length() const {
  return pimpl_->content.length();
}

RENITY_API void StringBuffer::load(SDL_RWops *src) {
  pimpl_->content.clear();

  if (src) {
    char *bufPtr = nullptr;
    Sint64 bufSize = RENITY_ReadCharBuffer(src, &bufPtr);
    if (bufPtr) {
      pimpl_->content = bufPtr;
      SDL_free(bufPtr);
    } else {
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "StringBuffer::load: Invalid RWops (%li).", bufSize);
    }
  }
}
}  // namespace renity
