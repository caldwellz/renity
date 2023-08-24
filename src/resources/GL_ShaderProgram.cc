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
#include "HashTable.h"
#include "ResourceManager.h"
#include "gl3.h"
#include "resources/GL_FragShader.h"
#include "resources/GL_VertShader.h"

constexpr size_t INFO_LOG_SIZE = 256;
static GLchar infoLog[INFO_LOG_SIZE];

namespace renity {
GL_ShaderProgram* currentGLShaderProgram = nullptr;

struct GL_ShaderProgram::Impl {
  explicit Impl()
      : dirty(false),
        valid(false),
        nextBindingPoint(1),
        blendSrc(GL_SRC_ALPHA),
        blendDst(GL_ONE_MINUS_SRC_ALPHA) {
    shaderProgram = glCreateProgram();
    if (!shaderProgram) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Shader(): GL error %i while creating shader program",
                   glGetError());
      return;
    }
    uniformBuffers[0] = 0;
    glGenBuffers(MAX_UNIFORM_BLOCK_NAMES, &uniformBuffers[1]);
    for (Uint32 buf = 1; buf <= MAX_UNIFORM_BLOCK_NAMES; ++buf) {
      if (uniformBuffers[buf] == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "GL_Shader(): Failed to generate a uniform buffer for "
                     "binding point %u",
                     buf);
        return;
      }
    }
  }

  ~Impl() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(MAX_UNIFORM_BLOCK_NAMES, &uniformBuffers[1]);
  }

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

    // Relinking clears the uniform buffer binding points; must rebind them
    bindingNames.enumerate([shaderProgram = this->shaderProgram,
                            &bindingPoints =
                                this->bindingPoints](const String& name) {
      GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, name.c_str());

      // If the underlying shaders changed, they may need different uniforms
      if (blockIndex == GL_INVALID_INDEX) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "GL_ShaderProgram::linkProgram: GL error %i occured "
                    "while rebinding block '%s'; clearing uniform bindings.",
                    glGetError(), name.c_str());
        bindingPoints.clear();
        return false;
      }
      GLuint bindingPoint = bindingPoints.get(name);
      glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
      return true;
    });
  }

  bool dirty, valid;
  GLuint shaderProgram, nextBindingPoint,
      uniformBuffers[MAX_UNIFORM_BLOCK_NAMES + 1];
  GLenum blendSrc, blendDst;
  GL_VertShaderPtr vert;
  GL_FragShaderPtr frag;
  HashTable<String, GLuint> bindingPoints;
  HashTable<GLuint, String> bindingNames;
};

RENITY_API GL_ShaderProgram::GL_ShaderProgram() { pimpl_ = new Impl(); }

RENITY_API GL_ShaderProgram::~GL_ShaderProgram() { delete pimpl_; }

RENITY_API void GL_ShaderProgram::activate() {
  currentGLShaderProgram = this;
  if (pimpl_->dirty) {
    pimpl_->linkProgram();
  }
#ifdef RENITY_DEBUG
  if (!pimpl_->valid) {
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_APPLICATION,
        "GL_ShaderProgram::use: Attempted to use invalid shader program %i",
        pimpl_->shaderProgram);
  }
#endif
  glBlendFunc(pimpl_->blendSrc, pimpl_->blendDst);
  glUseProgram(pimpl_->shaderProgram);
  for (GLuint bindPoint = 1; bindPoint < pimpl_->nextBindingPoint;
       ++bindPoint) {
    glBindBufferBase(GL_UNIFORM_BUFFER, bindPoint,
                     pimpl_->uniformBuffers[bindPoint]);
  }
}

RENITY_API GL_ShaderProgram* GL_ShaderProgram::getActive() {
  return currentGLShaderProgram;
}

static void flagReload(void* userdata) {
  GL_ShaderProgram::Impl* pimpl_ =
      static_cast<GL_ShaderProgram::Impl*>(userdata);
  pimpl_->dirty = true;
}

RENITY_API void GL_ShaderProgram::setBlendFunc(Uint32 src, Uint32 dest) {
  pimpl_->blendSrc = src;
  pimpl_->blendDst = dest;
}

template <typename T>
RENITY_API bool GL_ShaderProgram::setUniformBlock(String blockName,
                                                  Vector<T> uniforms) {
#ifdef RENITY_DEBUG
  // Sanity checks
  if (!pimpl_->valid) return false;
  if (uniforms.size() > MAX_UNIFORM_BLOCK_ITEMS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_ShaderProgram::setUniformBlock: Only %u uniforms are "
                 "allowed, but %li were passed",
                 MAX_UNIFORM_BLOCK_ITEMS, uniforms.size());
    return false;
  }
#endif

  // Associate binding points with shader program uniform block names, as needed
  // (e.g. "MyBlock" in "layout (std140) uniform MyBlock { vec4 myVec; }")
  if (!pimpl_->bindingPoints.exists(blockName)) {
    activate();
    GLuint blockIndex =
        glGetUniformBlockIndex(pimpl_->shaderProgram, blockName.c_str());
#ifdef RENITY_DEBUG
    if (blockIndex == GL_INVALID_INDEX) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_ShaderProgram::setUniformBlock: GL error %i occured "
                   "during block index search for '%s'",
                   glGetError(), blockName.c_str());
      return false;
    }
    if (pimpl_->nextBindingPoint > MAX_UNIFORM_BLOCK_NAMES) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_ShaderProgram::setUniformBlock: No unused "
                   "uniform binding points available for '%s'",
                   blockName.c_str());
      return false;
    }
#endif
    GLuint bindingPoint = pimpl_->nextBindingPoint++;
    glUniformBlockBinding(pimpl_->shaderProgram, blockIndex, bindingPoint);
    pimpl_->bindingPoints.put(blockName, bindingPoint);
    pimpl_->bindingNames.put(bindingPoint, blockName);
  }

  // Bind and fill the uniform buffer
  GLuint bindPoint = pimpl_->bindingPoints.get(blockName);
  glBindBuffer(GL_UNIFORM_BUFFER, pimpl_->uniformBuffers[bindPoint]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * uniforms.size(), uniforms.data(),
               GL_DYNAMIC_DRAW);
  // No need to UNbind the uniform buffer, since it won't associate with a VAO

  return true;
}
// Supported uniform type specializations
template RENITY_API bool GL_ShaderProgram::setUniformBlock(
    String blockName, Vector<vec4> uniforms);
template RENITY_API bool GL_ShaderProgram::setUniformBlock(
    String blockName, Vector<float> uniforms);
template RENITY_API bool GL_ShaderProgram::setUniformBlock(
    String blockName, Vector<Sint32> uniforms);
template RENITY_API bool GL_ShaderProgram::setUniformBlock(
    String blockName, Vector<unsigned int> uniforms);

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
