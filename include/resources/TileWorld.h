/****************************************************
 * TileWorld.h: Tile world resource                 *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Point2D.h"
#include "Resource.h"
#include "types.h"

namespace renity {
class RENITY_API TileWorld : public Resource {
 public:
  TileWorld();
  ~TileWorld();

  /** Draw the world at the given camera position.
   * \param cameraPos Top-left-relative world coordinates to center and draw at.
   * \param scale The scale to draw at, relative to the original tile size.
   */
  void draw(const Point2Di32 cameraPos, float scale = 1.0f);

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  struct Impl;
  Impl* pimpl_;
};
using TileWorldPtr = SharedPtr<TileWorld>;
}  // namespace renity
