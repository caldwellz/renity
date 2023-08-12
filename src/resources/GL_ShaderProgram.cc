/****************************************************
 * GL_ShaderProgram.cc: GL shader program resource  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/GL_ShaderProgram.h"

#include <SDL3/SDL_log.h>

#include "Dictionary.h"
#include "ResourceManager.h"
#include "gl3.h"
#include "resources/GL_FragShader.h"
#include "resources/GL_VertShader.h"

constexpr size_t INFO_LOG_SIZE = 256;
static GLchar infoLog[INFO_LOG_SIZE];

namespace renity {
struct GL_ShaderProgram::Impl {
  explicit Impl() : dirty(false), valid(false) {
    shaderProgram = glCreateProgram();
    if (!shaderProgram) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Shader(): GL error %i while creating shader program",
                   glGetError());
      return;
    }
  }

  ~Impl() { glDeleteProgram(shaderProgram); }

  void linkProgram() {
    dirty = false;
    if (!vert->isValid() || !frag->isValid()) {
      SDL_LogWarn(
          SDL_LOG_CATEGORY_APPLICATION,
          "GL_ShaderProgram::linkProgram: Unable to link invalid shader(s)");
      return;
    }

    GLint success;
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      valid = false;
      glGetProgramInfoLog(shaderProgram, INFO_LOG_SIZE, nullptr, infoLog);
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_ShaderProgram::linkProgram: Shader program %i failed to "
                   "link: '%s'",
                   shaderProgram, infoLog);
      return;
    }
    valid = true;
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_APPLICATION,
        "GL_ShaderProgram::linkProgram: Shader program %i linked successfully.",
        shaderProgram);
  }

  bool dirty, valid;
  GLuint shaderProgram;
  GL_VertShaderPtr vert;
  GL_FragShaderPtr frag;
};

RENITY_API GL_ShaderProgram::GL_ShaderProgram() { pimpl_ = new Impl(); }

RENITY_API GL_ShaderProgram::~GL_ShaderProgram() { delete pimpl_; }

static void flagReload(void* userdata) {
  GL_ShaderProgram::Impl* pimpl_ =
      static_cast<GL_ShaderProgram::Impl*>(userdata);
  pimpl_->dirty = true;
}

RENITY_API void GL_ShaderProgram::use() {
  if (pimpl_->dirty) {
    pimpl_->linkProgram();
  }

  if (!pimpl_->valid) {
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_APPLICATION,
        "GL_ShaderProgram::use: Attempted to use invalid shader program %i",
        pimpl_->shaderProgram);
    return;
  }

  glUseProgram(pimpl_->shaderProgram);
}

RENITY_API void GL_ShaderProgram::load(SDL_RWops* src) {
  const char *vertPath = "<undefined>", *fragPath = vertPath;
  Dictionary details;
  details.load(src);

  if (!details.get<const char*>("vertexShaderPath", &vertPath) ||
      !details.get<const char*>("fragmentShaderPath", &fragPath)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_ShaderProgram::load: Invalid shader details (missing "
                 "vertexShaderPath [%s] and/or fragmentShaderPath [%s])",
                 vertPath, fragPath);
    return;
  }

  // Usually we're changing files; detach any loaded shaders from the program
  if (pimpl_->vert) {
    glDetachShader(pimpl_->shaderProgram, pimpl_->vert->getShaderIndex());
  }
  if (pimpl_->frag) {
    glDetachShader(pimpl_->shaderProgram, pimpl_->frag->getShaderIndex());
  }

  // Once replaced, resource management will delete any now-detached shaders
  pimpl_->vert = ResourceManager::getActive()->get<GL_VertShader>(vertPath);
  pimpl_->vert->setReloadCallback(flagReload, pimpl_);
  glAttachShader(pimpl_->shaderProgram, pimpl_->vert->getShaderIndex());
  pimpl_->frag = ResourceManager::getActive()->get<GL_FragShader>(fragPath);
  pimpl_->frag->setReloadCallback(flagReload, pimpl_);
  glAttachShader(pimpl_->shaderProgram, pimpl_->frag->getShaderIndex());
  pimpl_->dirty = true;
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_ShaderProgram::load: (Re)linking shader program %i using "
                 "vertShader:[%s], fragShader:[%s])",
                 pimpl_->shaderProgram, vertPath, fragPath);
  pimpl_->linkProgram();
}
}  // namespace renity
