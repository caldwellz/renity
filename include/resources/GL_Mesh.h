/****************************************************
 * GL_Mesh.h: GL vertex buffer resource             *
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
struct MeshPosition {
  float x, y, z;
  Uint32 u, v;
};

class RENITY_API GL_Mesh : public Resource {
 public:
  GL_Mesh();
  ~GL_Mesh();

  /** Enable or disable wireframe drawing mode for EVERY mesh.
   * It's disabled by default (drawing in "fill" mode).
   */
  static void enableWireframe(bool enable = true);

  /** Draw mesh instances using the current texture and shader program.
   * Changes the currently-bound VAO/VBOs and does not restore them.
   * \param instances A vector of MeshPosition structures to draw.
   */
  void draw(const Vector<MeshPosition>& instances);

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  struct Impl;
  Impl* pimpl_;
};
using GL_MeshPtr = SharedPtr<GL_Mesh>;
}  // namespace renity
