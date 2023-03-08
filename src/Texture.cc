/***************************************************
 * Texture.cc: Texture management class             *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Texture.h"

#include <SDL3/SDL.h>

#include "Window.h"
#include "config.h"
#include "types.h"
#include "utils/physfsrwops.h"
#include "utils/surface_utils.h"
#include "utils/texture_utils.h"

namespace renity {
struct Texture::Impl {
  /** Defaults to full-screen VSynced mode at the native screen resolution. */
  Impl() {
    renderer = nullptr;
    surf = nullptr;
    tex = nullptr;
    colorKeyPosition.x = colorKeyPosition.y = -1;
    colorKeyEnabled = 0;
  }

  SDL_Renderer *renderer;
  SDL_Surface *surf;
  SDL_Texture *tex;
  SDL_Point colorKeyPosition;
  int colorKeyEnabled;
};

RENITY_API Texture::Texture() {
  // Initialize with default settings
  pimpl_ = new Impl();
}

RENITY_API Texture::Texture(const Window &window) {
  pimpl_ = new Impl();
  pimpl_->renderer = window.getRenderer();
}

RENITY_API Texture::Texture(const Window &window, const char *path) {
  pimpl_ = new Impl();
  pimpl_->renderer = window.getRenderer();
  load(PHYSFSRWOPS_openRead(path));
}

RENITY_API Texture::~Texture() {
  unload();
  delete pimpl_;
}

RENITY_API void Texture::load(SDL_RWops *src) {
  unload();
  if (!src) return;

  pimpl_->surf = RENITY_LoadPhysSurfaceRW(src);
  if (!pimpl_->renderer) {
    Window *w = Window::getActive();
    if (w) {
      pimpl_->renderer = w->getRenderer();
    }
  }
  reloadFromSurface();
}

RENITY_API void Texture::unload() {
  if (pimpl_->tex) {
    SDL_DestroyTexture(pimpl_->tex);
    pimpl_->tex = nullptr;
  }
  if (pimpl_->surf) {
    SDL_DestroySurface(pimpl_->surf);
    pimpl_->surf = nullptr;
  }
}

RENITY_API bool Texture::enableColorKey(const Point2Di &keyPosition) {
  pimpl_->colorKeyEnabled = 1;
  pimpl_->colorKeyPosition = keyPosition.toSDLPoint();
  if (isValid()) return reloadFromSurface();
  return true;
}

RENITY_API void Texture::disableColorKey() {
  pimpl_->colorKeyEnabled = 0;
  pimpl_->colorKeyPosition.x = pimpl_->colorKeyPosition.y = -1;
  reloadFromSurface();
}

RENITY_API bool Texture::isColorKeyEnabled() {
  return (pimpl_->colorKeyEnabled > 0);
}

RENITY_API bool Texture::isValid() const {
  return (pimpl_->tex && pimpl_->renderer);
}

// RENITY_API String Texture::getImagePath() const { return pimpl_->imgPath; }

RENITY_API Dimension2Di Texture::getSize() const {
  if (isValid()) {
    int w, h;
    if (0 == SDL_QueryTexture(pimpl_->tex, NULL, NULL, &w, &h))
      return Dimension2Di(w, h);
  }

  return Dimension2Di(0, 0);
}

RENITY_API bool Texture::setWindow(const Window &window) {
  if (pimpl_->renderer == window.getRenderer()) {
    return true;
  }
  pimpl_->renderer = window.getRenderer();
  return reloadFromSurface();
}

RENITY_API bool Texture::draw(const Rect2Di *source, const Rect2Di *dest,
                              const double &angle, const Point2Di *origin,
                              const bool &flipHorizontal,
                              const bool &flipVertical) {
  if (!isValid()) {
    return false;
  }

  SDL_FRect srcRect, destRect, *srcRectPtr = NULL, *destRectPtr = NULL;
  SDL_FPoint center, *centerPtr = NULL;
  SDL_RendererFlip flip = SDL_FLIP_NONE;

  if (source) {
    srcRect = source->toSDLFRect();
    srcRectPtr = &srcRect;
  }
  if (dest) {
    destRect = dest->toSDLFRect();
    destRectPtr = &destRect;
  }
  if (origin) {
    center = origin->toSDLFPoint();
    centerPtr = &center;
  }
  if (flipHorizontal) {
    flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (flipVertical) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
  }

  return (0 == SDL_RenderTextureRotated(pimpl_->renderer, pimpl_->tex,
                                        srcRectPtr, destRectPtr, angle,
                                        centerPtr, flip));
}

RENITY_API bool Texture::reloadFromSurface() {
  pimpl_->tex = RENITY_CreateTextureFromSurfaceEx(
      pimpl_->renderer, pimpl_->surf, pimpl_->colorKeyEnabled,
      &pimpl_->colorKeyPosition);
  return pimpl_->tex != NULL;
}
}  // namespace renity
