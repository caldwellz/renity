/****************************************************
 * GL_Shader.cc: GL shader program resource  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/GL_Shader.h"

#include <SDL3/SDL_log.h>

#include "gl3.h"
#include "resources/StringBuffer.h"

constexpr size_t INFO_LOG_SIZE = 256;
static GLchar infoLog[INFO_LOG_SIZE];

namespace renity {
struct GL_Shader::Impl {
  explicit Impl(GLenum shaderType) : valid(false) {
    if (shaderType != GL_VERTEX_SHADER && shaderType != GL_FRAGMENT_SHADER) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Shader(): Unsupported shader type %i", shaderType);
      shader = 0;
      return;
    }
    shader = glCreateShader(shaderType);
    if (!shader) {
      SDL_LogError(
          SDL_LOG_CATEGORY_APPLICATION,
          "GL_Shader(): GL error %i while creating type %i shader object",
          glGetError(), shaderType);
      return;
    }
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Shader(): Successfully created shader %i with type %i",
                   shader, shaderType);
  }

  ~Impl() { glDeleteShader(shader); }

  bool valid;
  GLuint shader;
};

RENITY_API GL_Shader::GL_Shader(GLenum shaderType) {
  pimpl_ = new Impl(shaderType);
}

RENITY_API GL_Shader::~GL_Shader() { delete pimpl_; }

RENITY_API GLuint GL_Shader::getShaderIndex() { return pimpl_->shader; }

RENITY_API bool GL_Shader::isValid() { return pimpl_->valid; }

RENITY_API void GL_Shader::load(SDL_RWops* src) {
  if (!pimpl_->shader) {
    return;
  }

  GLint success;
  StringBuffer buf;
  buf.load(src);
  const char* srcCode = buf.getCStr();
  glShaderSource(pimpl_->shader, 1, &srcCode, nullptr);
  glCompileShader(pimpl_->shader);
  glGetShaderiv(pimpl_->shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    pimpl_->valid = false;
    glGetShaderInfoLog(pimpl_->shader, INFO_LOG_SIZE, nullptr, infoLog);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Shader::load: Shader compilation failed: '%s'", infoLog);
    return;
  }
  pimpl_->valid = true;
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Shader::load: Successfully (re)compiled shader %i",
                 pimpl_->shader);
}
}  // namespace renity
