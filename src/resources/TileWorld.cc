/****************************************************
 * TileWorld.cc: Tile world resource                *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/TileWorld.h"

#include <SDL3/SDL_log.h>

#include "Dictionary.h"
#include "Dimension2D.h"
#include "GL_TileRenderer.h"
#include "Rect2D.h"
#include "ResourceManager.h"
#include "Window.h"
#include "gl3.h"
#include "resources/Tilemap.h"
#include "utils/string_helpers.h"

namespace renity {
struct MapInstance {
  MapInstance(const TilemapPtr &mapPtr, const Rect2Di32 &bounds)
      : map(mapPtr), worldBounds(bounds) {}
  TilemapPtr map;
  Rect2Di32 worldBounds;
};

struct TileWorld::Impl {
  explicit Impl() : prevPos(-1, -1) {}
  ~Impl() {}

  Point2Di32 prevPos;
  float prevScale;
  Vector<MapInstance> maps;
  GL_TileRenderer renderer;
};

RENITY_API TileWorld::TileWorld() { pimpl_ = new Impl(); }

RENITY_API TileWorld::~TileWorld() { delete pimpl_; }

RENITY_API void TileWorld::draw(const Point2Di32 cameraPos, float scale) {
  static Vector<MapInstance> visibleMaps;
  Dimension2Di32 windowSize = Window::getActive()->sizeInPixels();
  if (pimpl_->prevPos != cameraPos || scale != pimpl_->prevScale) {
    pimpl_->prevScale = scale;
    Rect2Di32 aabb = Rect2Di32::getFromCentroid(cameraPos, windowSize)
                         .scaleFromCenter(1.0f / scale);

    // TODO: Implement neighbor and/or binary search and map cache eviction
    visibleMaps.clear();
    for (auto inst : pimpl_->maps) {
      if (aabb.intersects(inst.worldBounds)) {
        visibleMaps.push_back(inst);
      }
    }
    pimpl_->prevPos = cameraPos;
  }

  for (auto instance : visibleMaps) {
    // Map inverts the Y axis into GL coordinates - no need to do it here
    Point2Di32 mapOffset = instance.worldBounds.position() - cameraPos;
    instance.map->draw(pimpl_->renderer, mapOffset);
  }
}

RENITY_API void TileWorld::load(SDL_RWops *src) {
  Impl *pimpl = pimpl_;
  Dictionary dict;
  dict.load(src);

  // (Re)load the map list
  pimpl_->maps.clear();
  dict.enumerateArray("maps", [pimpl](Dictionary &dict, const Uint32 &index) {
    const char *mapPath = "<undefined>";
    Sint32 x, y, width, height;
    if (!dict.get<const char *>("fileName", &mapPath) ||
        !dict.get<Sint32>("x", &x) || !dict.get<Sint32>("y", &y) ||
        !dict.get<Sint32>("width", &width) ||
        !dict.get<Sint32>("height", &height)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "TileWorld::load: Missing details for map '%s'", mapPath);
      return true;
    }

    TilemapPtr map = ResourceManager::getActive()->get<Tilemap>(mapPath);
    Rect2Di32 bounds(x, y, width, height);
    pimpl->maps.emplace_back(map, bounds);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "TileWorld::load: Successfully cached map '%s' with rect (%i, "
                 "%i)+(%i, %i).",
                 mapPath, x, y, width, height);
    return true;
  });

  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "TileWorld::load: Successfully loaded %u map(s).",
               pimpl->maps.size());
}
}  // namespace renity
