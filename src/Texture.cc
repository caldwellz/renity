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

// TODO: Add build-system support for the MSVC INCBIN tool.
#ifdef _MSC_VER
#ifndef __GNUC__
#include "resources/default_texture.h"
#else
#undef _MSC_VER
#endif
#endif
#ifndef RENITY_DEFAULT_TEXTURE
#define INCBIN_PREFIX p
#include "3rdparty/incbin/incbin.h"
INCBIN(DefaultTexture, "resources/default_texture.png");
#endif

namespace renity {
struct Texture::Impl {
  /** Defaults to full-screen VSynced mode at the native screen resolution. */
  Impl() {
    renderer = nullptr;
    surf = nullptr;
    tex = nullptr;
    lock = SDL_CreateMutex();
    colorKeyPosition.x = colorKeyPosition.y = -1;
    colorKeyEnabled = 0;
    wantTexUpdate = false;
  }

  ~Impl() { SDL_DestroyMutex(lock); }

  SDL_Renderer *renderer;
  SDL_Surface *surf;
  SDL_Texture *tex;
  SDL_mutex *lock;
  SDL_Point colorKeyPosition;
  int colorKeyEnabled;
  bool wantTexUpdate;
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
  // Load default not-found texture if not given a valid one
  if (!src) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "Texture::load: Invalid RWops - using default texture.\n");
    SDL_RWops *defSrc =
        SDL_RWFromConstMem(pDefaultTextureData, pDefaultTextureSize);
    // Shouldn't fail, but avoid infinite loops just to be safe
    if (defSrc) {
      load(defSrc);
    } else {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Texture::load: SDL_RWFromConstMem failed ('%s')\n",
                   SDL_GetError());
    }
    return;
  }

  SDL_LockMutex(pimpl_->lock);
  if (pimpl_->surf) {
    SDL_DestroySurface(pimpl_->surf);
  }
  pimpl_->surf = RENITY_LoadPhysSurfaceRW(src);
  if (!pimpl_->renderer) {
    Window *w = Window::getActive();
    if (w) {
      pimpl_->renderer = w->getRenderer();
    }
  }

  // Defer the actual update on hot-reloads, since in that case this is probably
  // being called from a filesystem watcher thread, **not the drawing thread**.
  if (pimpl_->tex)
    pimpl_->wantTexUpdate = true;
  else
    reloadFromSurface();
  SDL_UnlockMutex(pimpl_->lock);
}

RENITY_API void Texture::unload() {
  SDL_LockMutex(pimpl_->lock);
  if (pimpl_->tex) {
    SDL_DestroyTexture(pimpl_->tex);
    pimpl_->tex = nullptr;
  }
  if (pimpl_->surf) {
    SDL_DestroySurface(pimpl_->surf);
    pimpl_->surf = nullptr;
  }
  SDL_UnlockMutex(pimpl_->lock);
}

RENITY_API bool Texture::enableColorKey(const Point2Di &keyPosition) {
  SDL_LockMutex(pimpl_->lock);
  pimpl_->colorKeyEnabled = 1;
  pimpl_->colorKeyPosition = keyPosition.toSDLPoint();
  pimpl_->wantTexUpdate = true;
  SDL_UnlockMutex(pimpl_->lock);
  return true;
}

RENITY_API void Texture::disableColorKey() {
  SDL_LockMutex(pimpl_->lock);
  pimpl_->colorKeyEnabled = 0;
  pimpl_->colorKeyPosition.x = pimpl_->colorKeyPosition.y = -1;
  pimpl_->wantTexUpdate = true;
  SDL_UnlockMutex(pimpl_->lock);
}

RENITY_API bool Texture::isColorKeyEnabled() {
  return (pimpl_->colorKeyEnabled > 0);
}

RENITY_API bool Texture::isValid() const {
  SDL_LockMutex(pimpl_->lock);
  bool valid = pimpl_->tex && pimpl_->renderer;
  SDL_UnlockMutex(pimpl_->lock);
  return valid;
}

// RENITY_API String Texture::getImagePath() const { return pimpl_->imgPath; }

RENITY_API Dimension2Di Texture::getSize() const {
  Dimension2Di size(0, 0);
  SDL_LockMutex(pimpl_->lock);
  if (isValid()) {
    int w, h;
    if (0 == SDL_QueryTexture(pimpl_->tex, NULL, NULL, &w, &h))
      size = Dimension2Di(w, h);
  } else {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Texture::getSize() called on INVALID TEXTURE.\n");
  }
  SDL_UnlockMutex(pimpl_->lock);
  return size;
}

RENITY_API bool Texture::setWindow(const Window &window) {
  SDL_LockMutex(pimpl_->lock);
  if (pimpl_->renderer != window.getRenderer()) {
    pimpl_->renderer = window.getRenderer();
    pimpl_->wantTexUpdate = true;
  }
  SDL_UnlockMutex(pimpl_->lock);
  return true;
}

RENITY_API bool Texture::draw(const Rect2Di *source, const Rect2Di *dest,
                              const double &angle, const Point2Di *origin,
                              const bool &flipHorizontal,
                              const bool &flipVertical) {
  SDL_LockMutex(pimpl_->lock);
  if (!isValid()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Texture::draw: %s was not valid - loading default(s).\n",
                pimpl_->tex ? "RENDERER" : "TEXTURE");
    load(nullptr);
    //  SDL_UnlockMutex(pimpl_->lock);
    //  return false;
  }
  if (pimpl_->wantTexUpdate == true) reloadFromSurface();

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
  int result =
      SDL_RenderTextureRotated(pimpl_->renderer, pimpl_->tex, srcRectPtr,
                               destRectPtr, angle, centerPtr, flip);
  SDL_UnlockMutex(pimpl_->lock);
  return result == 0;
}

/** (Re)create the SDL texture.
 * This should ONLY be called from the main rendering thread. */
bool Texture::reloadFromSurface() {
  SDL_LockMutex(pimpl_->lock);
  if (pimpl_->tex) {
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "Texture::reloadFromSurface: Destroying old texture\n");
    SDL_DestroyTexture(pimpl_->tex);
  }
  pimpl_->tex = RENITY_CreateTextureFromSurfaceEx(
      pimpl_->renderer, pimpl_->surf, pimpl_->colorKeyEnabled,
      &pimpl_->colorKeyPosition);
  bool valid = pimpl_->tex != nullptr;
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "Texture:reloadFromSurface: New tex is valid: %s\n",
                 valid ? "TRUE" : "FALSE");
  pimpl_->wantTexUpdate = false;
  SDL_UnlockMutex(pimpl_->lock);
  return valid;
}
}  // namespace renity
