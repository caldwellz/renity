/****************************************************
 * GL_PointRenderer.cc: GL point instance renderer  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "GL_PointRenderer.h"

#include <SDL3/SDL_log.h>

#include "gl3.h"

namespace renity {
struct GL_PointRenderer::Impl {
  explicit Impl() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
  }

  ~Impl() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
  }

  GLuint vao, vbo;
};

RENITY_API GL_PointRenderer::GL_PointRenderer() {
  pimpl_ = new Impl();
  glBindVertexArray(pimpl_->vao);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->vbo);

  // Configure the points buffer that will be filled every draw call
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointInstance), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT, sizeof(PointInstance),
                         (void *)(offsetof(PointInstance, u)));
  glEnableVertexAttribArray(1);

  // Unbind everything to be safe
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  int maxPointSize[2] = {0, 0};
  glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, maxPointSize);
  SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Point sizes %u-%u avail",
               maxPointSize[0], maxPointSize[1]);
}

RENITY_API GL_PointRenderer::~GL_PointRenderer() { delete pimpl_; }

RENITY_API void GL_PointRenderer::draw(const Vector<PointInstance> &instances) {
  glBindVertexArray(pimpl_->vao);
  // TODO: Try removing this, since the VAO should bind it
  glBindBuffer(GL_ARRAY_BUFFER, pimpl_->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(PointInstance) * instances.size(),
               instances.data(), GL_STREAM_DRAW);
  glDrawArrays(GL_POINTS, 0, instances.size());
}
}  // namespace renity
