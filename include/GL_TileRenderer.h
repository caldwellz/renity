/****************************************************
 * GL_TileRenderer.h: GL tile instance renderer     *
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
struct TileInstance {
  Uint32 x, y, z, u, v;
};

class RENITY_API GL_TileRenderer {
 public:
  GL_TileRenderer();
  ~GL_TileRenderer();

  /** Enable or disable wireframe drawing mode for EVERY tile.
   * It's disabled by default (drawing in "fill" mode).
   */
  static void enableWireframe(bool enable = true);

  /** Draw a tile list using the current texture and shader program.
   * Changes the currently-bound VAO/VBOs and does not restore them.
   * \param instances A vector of TileInstance structures to draw.
   */
  void draw(const Vector<TileInstance>& instances);

 private:
  struct Impl;
  Impl* pimpl_;
};
}  // namespace renity
