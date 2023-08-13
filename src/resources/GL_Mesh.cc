/****************************************************
 * GL_Mesh.cc: GL vertex buffer resource            *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/GL_Mesh.h"

#include <SDL3/SDL_log.h>

#include "Dictionary.h"
#include "gl3.h"

constexpr size_t INFO_LOG_SIZE = 256;
static GLchar infoLog[INFO_LOG_SIZE];
static GLenum drawMode = GL_TRIANGLES;
static GLuint activeVao = 0;

namespace renity {
struct GL_Mesh::Impl {
  explicit Impl() : loaded(false), elementCount(0) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
  }

  ~Impl() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }

  bool loaded;
  GLuint vao, vbo, ebo;
  Uint32 elementCount;
};

RENITY_API GL_Mesh::GL_Mesh() { pimpl_ = new Impl(); }

RENITY_API GL_Mesh::~GL_Mesh() { delete pimpl_; }

RENITY_API void GL_Mesh::enableWireframe(bool enable) {
  // The typical way to do this in desktop GL is glPolygonMode(), but ES3
  // doesn't have that, so fake it using a line drawing mode
  drawMode = enable ? GL_LINES : GL_TRIANGLES;
}

RENITY_API void GL_Mesh::use() {
#ifdef RENITY_DEBUG
  if (!pimpl_->loaded) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "GL_Mesh::use: Attempted to use unloaded mesh %i", pimpl_->vao);
  }
  activeVao = pimpl_->vao;
#endif
  glBindVertexArray(pimpl_->vao);
}

RENITY_API void GL_Mesh::draw() {
#ifdef RENITY_DEBUG
  if (activeVao != pimpl_->vao) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "GL_Mesh::draw: Attempted to draw mesh %i when %i is in use",
                pimpl_->vao, activeVao);
  }
#endif
  glDrawElements(drawMode, pimpl_->elementCount, GL_UNSIGNED_INT, nullptr);
}

RENITY_API void GL_Mesh::load(SDL_RWops *src) {
  Vector<float> vertices;
  Dictionary details;
  details.load(src);

  // Load vertices, bailing out if there aren't any
  Uint32 vertCount = details.end("vertices");
  if (vertCount == 0 || vertCount == UINT32_MAX) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "GL_Mesh::load: No vertices found");
    return;
  }
  vertices.reserve(vertCount);
  details.enumerateArray("vertices",
                         [&vertices](Dictionary &dict, const Uint32 &index) {
                           float val;
                           if (dict.get<float>(nullptr, &val)) {
                             vertices.push_back(val);
                           }
                           return true;
                         });
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Mesh::load: Loaded %i of %i vertex floats",
                 vertices.size(), vertCount);

  // Load indices, treating vertex order as index order if there's no indices
  Vector<Uint32> indices;
  Uint32 indCount = details.end("indices");
  if (indCount == 0 || indCount == UINT32_MAX) {
    indCount = vertCount / 3;
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Mesh::load: No indices found; creating monotonic list");
    indices.reserve(indCount);
    for (Uint32 index = 0; index < indCount; ++index) {
      indices.push_back(index);
    }
  } else {
    indices.reserve(indCount);
    details.enumerateArray("indices",
                           [&indices](Dictionary &dict, const Uint32 &index) {
                             Uint32 val;
                             if (dict.get<Uint32>(nullptr, &val)) {
                               indices.push_back(val);
                             }
                             return true;
                           });
  }
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Mesh::load: Loaded %i of %i indices", indices.size(),
                 indCount);

  // Unload the mesh file and then buffer vector data into the GL
  details.load(nullptr);
  glBindVertexArray(pimpl_->vao);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->vbo);
  // TODO: Select the buffer usage more intelligently and/or with a field
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pimpl_->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint32) * indices.size(),
               indices.data(), GL_STATIC_DRAW);
  // glVertexAttribPointer "binds" the VBO/EBO to VAO attribute(s)
  // For a better explanation, see https://stackoverflow.com/a/59892245
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  pimpl_->elementCount = indCount;
  pimpl_->loaded = true;
}
}  // namespace renity
