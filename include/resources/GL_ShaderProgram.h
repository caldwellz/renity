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
// The GL guarantees >=24 binding points per program and up to 16kb block sizes:
// https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glGet.xhtml
constexpr size_t MAX_UNIFORM_BLOCK_NAMES = 24;
constexpr size_t MAX_UNIFORM_BLOCK_FLOATS = 16384 / sizeof(float);
using UniformArray = Vector<float>;

class RENITY_API GL_ShaderProgram : public Resource {
 public:
  GL_ShaderProgram();
  ~GL_ShaderProgram();

  /** Set a uniform block's buffer data, up to MAX_UNIFORM_BLOCK_FLOATS.
   * MAX_UNIFORM_BLOCK_NAMES specifies the max number of unique blockNames.
   * \returns True on success, false otherwise.
   */
  bool setUniformBlock(String blockName, UniformArray uniforms);

  /** Make this the active shader for the current GL context.
   * Also binds any previously-set uniform block buffers, and so must be called
   * AFTER any needed setUniformBlock() calls.
   */
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
