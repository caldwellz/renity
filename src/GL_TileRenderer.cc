/****************************************************
 * GL_TileRenderer.cc: GL tile instance renderer    *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "GL_TileRenderer.h"

#include "ResourceManager.h"
#include "gl3.h"

namespace renity {
static GLenum drawMode = GL_TRIANGLES;

struct GL_TileRenderer::Impl {
  explicit Impl() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    tileShader = ResourceManager::getActive()->get<GL_ShaderProgram>(
        "/assets/shaders/tile2d.shader");
  }

  ~Impl() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
  }

  GLuint vao, vbo, ibo;
  GL_ShaderProgramPtr tileShader;
};

RENITY_API GL_TileRenderer::GL_TileRenderer() {
  pimpl_ = new Impl();
  Vector<float> verticesWithUvs = {
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f,  0.0f, 1.0f, 1.0f};

  const size_t bufSize = sizeof(float) * verticesWithUvs.size();
  const size_t bufStride = sizeof(float) * 5;
  glBindVertexArray(pimpl_->vao);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->vbo);
  // TODO: Select the buffer usage more intelligently and/or with a field
  glBufferData(GL_ARRAY_BUFFER, bufSize, verticesWithUvs.data(),
               GL_STATIC_DRAW);

  // Configure and enable the interpretation of the vertex and UV attributes
  // glVertexAttribPointer also "binds" the VBO/EBO to VAO attribute(s)
  // For a better explanation, see https://stackoverflow.com/a/59892245
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, bufStride, 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, bufStride,
                        (const void *)(sizeof(float) * 3));

  // Configure the instances buffer that will be filled every draw call
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->ibo);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_UNSIGNED_INT, GL_FALSE, sizeof(TileInstance),
                        0);
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_UNSIGNED_INT, GL_FALSE, sizeof(TileInstance),
                        (const void *)(offsetof(TileInstance, t)));
  glVertexAttribDivisor(3, 1);

  // Unbind everything to be safe
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

RENITY_API GL_TileRenderer::~GL_TileRenderer() { delete pimpl_; }

RENITY_API void GL_TileRenderer::enableWireframe(bool enable) {
  // The typical way to do this in desktop GL is glPolygonMode(), but ES3
  // doesn't have that, so fake it using a line drawing mode
  drawMode = enable ? GL_LINES : GL_TRIANGLES;
}

RENITY_API GL_ShaderProgramPtr GL_TileRenderer::getTileShader() {
  return pimpl_->tileShader;
}

RENITY_API void GL_TileRenderer::draw(const Vector<TileInstance> &tiles) {
  pimpl_->tileShader->activate();
  glBindVertexArray(pimpl_->vao);
  // TODO: Try removing this, since the VAO should bind it
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->ibo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(TileInstance) * tiles.size(),
               tiles.data(), GL_STREAM_DRAW);
  glDrawArraysInstanced(drawMode, 0, 6, tiles.size());
}
}  // namespace renity
