/****************************************************
 * Tileset.h: Tileset resource                      *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Dimension2D.h"
#include "Resource.h"
#include "types.h"

namespace renity {
class RENITY_API Tileset : public Resource {
 public:
  Tileset();
  ~Tileset();

  /** Make this the active tileset for the current Window. */
  void use();

  /** Get the number of drawable tiles in each dimension (width and height). */
  Dimension2Du32 getTileCounts();

 protected:
  friend class ResourceManager;
  void load(SDL_RWops* src);

 private:
  struct Impl;
  Impl* pimpl_;
};
using TilesetPtr = SharedPtr<Tileset>;
}  // namespace renity
