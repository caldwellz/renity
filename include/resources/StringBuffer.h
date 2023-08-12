/****************************************************
 * StringBuffer.h: Fully-buffered String resource   *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Resource.h"
#include "types.h"

namespace renity {
class RENITY_API StringBuffer : public Resource {
 public:
  StringBuffer();
  ~StringBuffer();

  String getString() const;

  const char *getCStr() const;

  void load(SDL_RWops *src);

 private:
  struct Impl;
  Impl *pimpl_;
};
using StringBufferPtr = SharedPtr<StringBuffer>;
}  // namespace renity
