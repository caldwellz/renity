/****************************************************
 * GL_PointRenderer.h: GL point instance renderer   *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "types.h"

namespace renity {
struct PointInstance {
  // PointInstance() : x(0), y(0), z(0), u(0), v(0) {}
  float x, y, z;
  Uint32 u, v;
};

class RENITY_API GL_PointRenderer {
 public:
  GL_PointRenderer();
  ~GL_PointRenderer();

  /** Draw a point list using the current texture and shader program.
   * Changes the currently-bound VAO/VBOs and does not restore them.
   * \param instances A vector of PointInstance structures to draw.
   */
  void draw(const Vector<PointInstance>& instances);

 private:
  struct Impl;
  Impl* pimpl_;
};
}  // namespace renity
