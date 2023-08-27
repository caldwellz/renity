/****************************************************
 * Tilemap.cc: Tilemap resource                     *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/Tilemap.h"

#include <SDL3/SDL_log.h>
// #include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
// #include <SDL3/SDL_surface.h>

#include "Dictionary.h"
#include "Dimension2D.h"
#include "GL_TileRenderer.h"
#include "ResourceManager.h"
#include "gl3.h"
#include "resources/Tileset.h"
#include "utils/string_helpers.h"

namespace renity {
struct TilesetInstance {
  TileId firstGid;
  TilesetPtr tileset;
  Vector<TileInstance> tiles;
};

constexpr Uint8 MAX_DETAIL_VECS = 1 + MAX_MAP_LIGHTS * 2;
struct Tilemap::Impl {
  explicit Impl() : nextLightSlot(1) {
    mapDetails.assign(MAX_DETAIL_VECS, {0.0f, 0.0f, 0.0f, 0.0f});
  }
  ~Impl() {}

  Uint8 nextLightSlot;
  Dimension2Du32 pixelSize;
  Vector<vec4> mapDetails;
  Vector<TilesetInstance> tilesets;
};

RENITY_API Tilemap::Tilemap() { pimpl_ = new Impl(); }

RENITY_API Tilemap::~Tilemap() { delete pimpl_; }

RENITY_API void Tilemap::draw(GL_TileRenderer &renderer,
                              const Point2Di32 position) {
  // Set shader uniforms specifying map position, size, and depth.
  // The rest of the values are set during load().
  // Vertex shader will use this along with tile X/Y/Z to position & sort tiles.
  pimpl_->mapDetails[0].x = (float)position.x();
  pimpl_->mapDetails[0].y = position.y() * -1.0f;
  renderer.getTileShader()->setUniformBlock("MapDetails", pimpl_->mapDetails);

  for (auto &tsInstance : pimpl_->tilesets) {
    tsInstance.tileset->use();
    renderer.draw(tsInstance.tiles);
  }
}

RENITY_API void Tilemap::load(SDL_RWops *src) {
  Impl *pimpl = pimpl_;
  Dictionary dict;
  dict.load(src);

  Uint32 tileCountX = 0, tileCountY = 0, tileWidth = 0, tileHeight = 0;
  dict.get<Uint32>("width", &tileCountX);
  dict.get<Uint32>("height", &tileCountY);
  dict.get<Uint32>("tilewidth", &tileWidth);
  dict.get<Uint32>("tileheight", &tileHeight);
  pimpl_->pixelSize.width(tileCountX * tileWidth);
  pimpl_->pixelSize.height(tileCountY * tileHeight);
  if (!pimpl_->pixelSize.width() || !pimpl_->pixelSize.height()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Tilemap::load: Invalid map size with width:%u, height:%u, "
                 "tilewidth:%u, tileheight:%u",
                 tileCountX, tileCountY, tileWidth, tileHeight);
    return;
  }
  pimpl_->mapDetails.assign(MAX_DETAIL_VECS, {0.0f, 0.0f, 0.0f, 0.0f});

  // (Re)load the tilesets
  pimpl_->tilesets.clear();
  dict.enumerateArray(
      "tilesets", [pimpl](Dictionary &dict, const Uint32 &index) {
        TilesetInstance ts;
        const char *tilesetPath;
        if (!dict.get<TileId>("firstgid", &ts.firstGid) ||
            !dict.get<const char *>("source", &tilesetPath)) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                       "Tilemap::load: Missing tileset firstgid or source");
          return true;
        }
        ts.tileset = ResourceManager::getActive()->get<Tileset>(tilesetPath);
        pimpl->tilesets.push_back(ts);
        SDL_LogVerbose(
            SDL_LOG_CATEGORY_APPLICATION,
            "Tilemap::load: Successfully loaded tileset '%s' with firstgid %u.",
            tilesetPath, ts.firstGid);
        return true;
      });

  // (Re)load the tiles, indexed by tileset
  Uint32 layerCount = dict.end("layers");
  dict.enumerateArray("layers", [pimpl, layerCount, tileCountX, tileCountY,
                                 tileWidth, tileHeight](Dictionary &dict,
                                                        const Uint32 &index) {
    /*
    if (index >= MAX_MAP_LAYERS) {
      SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "Tilemap::load: Too many map layers - only %u are supported.",
        MAX_MAP_LAYERS);
      return false;
    }
    */
    // TODO: Add object layer support if/when needed
    const char *type = "<undefined>";
    if (!dict.get<const char *>("type", &type) ||
        !endsWith(type, "tilelayer")) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Tilemap::load: Unsupported tile layer type '%s'", type);
      return true;
    }
    const char *layerName = "<unnamed>";
    dict.get("name", &layerName);
    Uint32 layerId;
    dict.get("id", &layerId);

    // Load the list of tiles
    dict.select("data");
    Uint32 tileNum;
    for (tileNum = dict.begin(); tileNum < tileCountX * tileCountY; ++tileNum) {
      TileInstance tile;
      tile.v = 0;

      // Layers should be the same size as the map; ignore missing/extra tiles
      Uint32 tileId = 0;
      dict.getIndex(tileNum, &tileId);
      if (tileId == 0) {
        // Also ignore blank/empty tiles (indexes always start at 1)
        continue;
      }

      // Invert the Y into bottom-left coordinates for the shader
      // TODO: Find out whether tiles are ever *not* indexed from the top-left
      // TODO: Test and support variable tile sizes in the same map
      Uint32 mapSpaceX = tileNum % tileCountX;
      Uint32 mapSpaceY = tileNum / tileCountX;
      tile.x = mapSpaceX * tileWidth;
      tile.y = (tileCountY - 1 - mapSpaceY) * tileHeight;

      // Tiled layer order is currently bottom-to-top; top layers have lowest Z
      Uint32 layerZ = (layerCount - layerId) * pimpl->pixelSize.height();

      // Currently we're assuming a top-down view with tiles now drawn relative
      // to the bottom-left; so, lower Y means draw in front (i.e. lower Z)
      tile.z = layerZ + tile.y;

      // Figure out which tileset the tile belongs to
      Sint32 tilesetIndex;
      // Tilesets should be in firstgid order, so check ranges from the end
      for (tilesetIndex = pimpl->tilesets.size() - 1; tilesetIndex >= 0;
           --tilesetIndex) {
        Uint32 firstGid = pimpl->tilesets[tilesetIndex].firstGid;
        if (firstGid <= tileId) {
          // Make it a 0-index into the specific tileset
          tileId -= firstGid;
          break;
        }
      }
      if (tilesetIndex < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Tilemap::load: Tileset not found for tile (%u, %u) on "
                     "layer %u ('%s').",
                     mapSpaceX, mapSpaceY, layerId, layerName);
        continue;
      }

      // Add its pointLight if it has one
      Uint32 lightColor =
          pimpl->tilesets[tilesetIndex].tileset->getLightColor(tileId);
      if (lightColor != 0) {
        if (pimpl->nextLightSlot >= MAX_DETAIL_VECS) {
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                      "Tilemap::load: Exceeded MAX_MAP_LIGHTS (%u); skipping "
                      "light at (%u, %u) on layer '%s'",
                      MAX_MAP_LIGHTS, mapSpaceX, mapSpaceY, layerName);
        } else {
          vec4 lightColorVec;
          lightColorVec.r = (float)((lightColor >> 24) & 0x000000FF) / 255.0f;
          lightColorVec.g = (float)((lightColor >> 16) & 0x000000FF) / 255.0f;
          lightColorVec.b = (float)((lightColor >> 8) & 0x000000FF) / 255.0f;
          lightColorVec.a = (float)(lightColor & 0x000000FF) / 255.0f;
          pimpl->mapDetails.at(pimpl->nextLightSlot++) = lightColorVec;
          vec4 lightPos;
          lightPos.x = (float)tile.x;
          lightPos.y = (float)tile.y;
          lightPos.z = (float)tile.z;
          lightPos.w = 1.0f;
          pimpl->mapDetails.at(pimpl->nextLightSlot++) = lightPos;
        }
      }

      // Convert tile id to U/V position and invert the Y to make it bottup-up
      Dimension2Du32 tilesetDims =
          pimpl->tilesets[tilesetIndex].tileset->getTileCounts();
      Uint32 tilesetSpaceT = tileId % tilesetDims.width();
      Uint32 tilesetSpaceU = tileId / tilesetDims.width();
      tile.t = tilesetSpaceT * tileWidth;
      tile.u = (tilesetDims.height() - 1 - tilesetSpaceU) * tileHeight;

      SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                     "Tile (%u, %u, %u)[%u, %u] -> (%u, %u, %u)[%u, %u]",
                     mapSpaceX, mapSpaceY, layerId, tilesetSpaceT,
                     tilesetSpaceU, tile.x, tile.y, tile.z, tile.t, tile.u);
      pimpl->tilesets[tilesetIndex].tiles.push_back(tile);
    }
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_APPLICATION,
        "Tilemap::load: Successfully loaded layer %u ('%s') of type '%s' "
        "with %u of %u total tiles.",
        layerId, layerName, type, tileNum, dict.end());

    return true;
  });

  // Preconfigure MapDetails for shader
  pimpl_->mapDetails[0].s = -(float)pimpl_->pixelSize.height();
  pimpl_->mapDetails[0].t = (float)(layerCount * pimpl_->pixelSize.height());

  // TODO: Sort tiles front-to-back to take advantage of the depth buffer

  SDL_LogVerbose(
      SDL_LOG_CATEGORY_APPLICATION,
      "Tilemap::load: Successfully loaded %ux%u px map with %u layer(s), "
      "%u tileset(s).",
      pimpl_->pixelSize.width(), pimpl_->pixelSize.height(), layerCount,
      pimpl_->tilesets.size());
}
}  // namespace renity
