/****************************************************
 * GL_ShaderProgram.h: GL shader program resource   *
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
class RENITY_API GL_ShaderProgram : public Resource {
 public:
  GL_ShaderProgram();
  ~GL_ShaderProgram();

  /** Make this the active shader for the current GL context. */
  void use();

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  friend static void flagReload(void* userdata);
  struct Impl;
  Impl* pimpl_;
};
using GL_ShaderProgramPtr = SharedPtr<GL_ShaderProgram>;
}  // namespace renity
