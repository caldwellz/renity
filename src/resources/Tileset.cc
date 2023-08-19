/****************************************************
 * Tileset.cc: Tileset resource                     *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/Tileset.h"

#include <SDL3/SDL_log.h>
// #include <SDL3/SDL_pixels.h>
// #include <SDL3/SDL_surface.h>

#include <cmath>

#include "Dictionary.h"
#include "ResourceManager.h"
#include "resources/GL_ShaderProgram.h"
#include "resources/GL_Texture2D.h"

namespace renity {
struct Tileset::Impl {
  explicit Impl() {}
  ~Impl() {}

  Dimension2Du32 tileCount;
  Vector<float> tilesetSize;
  GL_Texture2DPtr tex;
};

RENITY_API Tileset::Tileset() { pimpl_ = new Impl(); }

RENITY_API Tileset::~Tileset() { delete pimpl_; }

RENITY_API void Tileset::use() {
  if (!pimpl_->tex) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Tileset::use: Texture has not been loaded");
    return;
  }
  pimpl_->tex->use();

  // Set shader uniforms specifying the tileset width and height increments.
  // Vertex shader will use this along with tile UVs to locate tiles.
  GL_ShaderProgram::getActive()->setUniformBlock("TilesetDetails",
                                                 pimpl_->tilesetSize);
}

RENITY_API Dimension2Du32 Tileset::getTileCounts() { return pimpl_->tileCount; }

RENITY_API void Tileset::load(SDL_RWops* src) {
  Dictionary dict;
  dict.load(src);

  // TODO: Do we need transparency mapping? Even quantized PNGs should be able
  // to use a palette index for a fully-transparent background color
  /*
    const char* tpColorStr;
    if (dict.get<const char*>("transparentcolor", &tpColorStr)) {
      Uint8 r, g, b;
      Uint32 tpColorIn = std::stol(tpColorStr.substr(1), nullptr, 16);
      r = (tpColorIn & 0x00FF0000) >> 16;
      g = (tpColorIn & 0x0000FF00) >> 8;
      b = (tpColorIn & 0x000000FF);
      Uint32 tpColorMapped = SDL_MapRGB(surf->format, r, g, b);
      SDL_SetSurfaceColorKey(surf, SDL_TRUE, tpColorMapped);
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "Tileset::load: Got transparency key 0x%06x from 0x%06x",
                   tpColorMapped, tpColorIn);
    }
  */

  const char* sheetPath;
  Uint32 sheetWidth, sheetHeight, tileWidth, tileHeight;
  if (!dict.get<const char*>("image", &sheetPath) ||
      !dict.get<Uint32>("imagewidth", &sheetWidth) ||
      !dict.get<Uint32>("imageheight", &sheetHeight) ||
      !dict.get<Uint32>("tilewidth", &tileWidth) ||
      !dict.get<Uint32>("tileheight", &tileHeight)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Tileset::load: Missing image path or dimension details");
    return;
  }

  pimpl_->tex = ResourceManager::getActive()->get<GL_Texture2D>(sheetPath);
  Dimension2Du32 imgSize = pimpl_->tex->getSize();
  if (imgSize.width() != sheetWidth || imgSize.height() != sheetHeight) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Tileset::load: Size mismatch (%ux%u vs. %ux%u) between "
                "tileset and image [%s]",
                sheetWidth, sheetHeight, imgSize.width(), imgSize.height(),
                sheetPath);
  }

  pimpl_->tilesetSize = {(float)tileWidth, (float)tileHeight,
                         (float)imgSize.width(), (float)imgSize.height()};
  pimpl_->tileCount.width(imgSize.width() / tileWidth);
  pimpl_->tileCount.height(imgSize.height() / tileHeight);
}
}  // namespace renity
