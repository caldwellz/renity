/****************************************************
 * Tilemap.h: Tilemap resource                      *
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
// constexpr Uint8 MAX_MAP_LAYERS = 64;

class RENITY_API Tilemap : public Resource {
 public:
  Tilemap();
  ~Tilemap();

  /** Draw the map using a square mesh and special shader program.
   * \param position A top-left-relative screen location to draw at, in pixels.
   */
  void draw(const Point2Di32 position);

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  struct Impl;
  Impl* pimpl_;
};
using TilemapPtr = SharedPtr<Tilemap>;
}  // namespace renity
