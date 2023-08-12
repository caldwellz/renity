/****************************************************
 * GL_Shader.h: OpenGL shader loader interface      *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Resource.h"
#include "gl3.h"
#include "types.h"

namespace renity {
class RENITY_API GL_Shader : public Resource {
 public:
  /** Subclasses should initialize this with their specific shader type */
  GL_Shader(GLenum shaderType);
  GL_Shader() = delete;
  ~GL_Shader();

  /** Get the shader object number.
   * \returns >0 if a shader was successfully created; 0 otherwise.
   */
  GLuint getShaderIndex();

  /** Get the shader validity.
   * \returns True if the shader was successfully compiled; false otherwise.
   */
  bool isValid();

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  struct Impl;
  Impl* pimpl_;
};
using GL_ShaderPtr = SharedPtr<GL_Shader>;
}  // namespace renity
