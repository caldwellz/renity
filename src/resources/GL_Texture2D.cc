/****************************************************
 * GL_Texture2D.cc: 2D texture resource class       *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include "resources/GL_Texture2D.h"

#include <SDL3/SDL_image.h>

#include "config.h"
#include "gl3.h"
#include "types.h"
#include "utils/surface_utils.h"

// TODO: Add build-system support for the MSVC INCBIN tool.
#ifdef _MSC_VER
#ifndef __GNUC__
#include "resources/default/texture.h"
#endif
#endif
#ifndef RENITY_DEFAULT_TEXTURE
#define INCBIN_PREFIX p
#include "3rdparty/incbin/incbin.h"
INCBIN(DefaultGL_Texture2D, "resources/default_texture.png");
#endif

namespace renity {
struct GL_Texture2D::Impl {
  Impl() : size(0, 0), texUnit(GL_TEXTURE0) { glGenTextures(1, &tex); }

  ~Impl() { glDeleteTextures(1, &tex); }

  GLuint tex;
  GLenum texUnit;
  Dimension2Du32 size;
};

RENITY_API GL_Texture2D::GL_Texture2D() { pimpl_ = new Impl(); }

RENITY_API GL_Texture2D::~GL_Texture2D() { delete pimpl_; }

RENITY_API void GL_Texture2D::load(SDL_RWops *src) {
  // Load default not-found texture if not given a valid one
  SDL_Surface *surf = RENITY_LoadPhysSurfaceRW(src);
  if (!surf) {
    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "GL_Texture2D::load: Invalid RWops - using default texture.\n");
    SDL_RWops *defSrc =
#ifdef RENITY_DEFAULT_TEXTURE
        SDL_RWFromConstMem(pDefaultTextureData, pDefaultTextureSize);
#else
        SDL_RWFromConstMem(pDefaultGL_Texture2DData, pDefaultGL_Texture2DSize);
#endif
    // Shouldn't fail, but avoid infinite loops just to be safe
    if (defSrc) {
      load(defSrc);
    } else {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "GL_Texture2D::load: SDL_RWFromConstMem failed ('%s')\n",
                   SDL_GetError());
    }
    return;
  }

  // Convert the pixel data from its original format to 32-bit RGBA.
  // Using RGBA32 instead of RGBA8888 converts from little-endian ABGR as needed
  // Image Y axes also need to be flipped into GL's bottom-left coordinates.
  SDL_Surface *rgbaSurf = RENITY_FlipSurfaceVertical(
      SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32), SDL_TRUE);
  SDL_DestroySurface(surf);
  if (!rgbaSurf) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Texture2D::load: Surface format conversion failed: '%s'",
                 SDL_GetError());
    return;
  }
  pimpl_->size.width(rgbaSurf->w);
  pimpl_->size.height(rgbaSurf->h);

  // Bind/configure/upload the texture data and auto-generate mipmaps
  glBindTexture(GL_TEXTURE_2D, pimpl_->tex);
  // TODO: Make the texture wrapping/filtering options configurable
  // GL_BORDER mode is not available in base ES3, so we'll default to GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbaSurf->w, rgbaSurf->h, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, rgbaSurf->pixels);
  SDL_DestroySurface(rgbaSurf);
  glGenerateMipmap(GL_TEXTURE_2D);
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Texture2D::load: Successfully buffered %ux%u texture",
                 pimpl_->size.width(), pimpl_->size.height());
}

RENITY_API void GL_Texture2D::setTextureUnit(Uint32 unit) {
#ifdef RENITY_DEBUG
  if ((unit >= MAX_TEXTURE_UNITS && unit < GL_TEXTURE0) ||
      (unit >= GL_TEXTURE0 + MAX_TEXTURE_UNITS)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "GL_Texture2D::setTextureUnit: Attempted to use invalid "
                 "texture unit %i",
                 unit);
    return;
  }
#endif
  if (unit >= GL_TEXTURE0 && unit < GL_TEXTURE0 + MAX_TEXTURE_UNITS) {
    pimpl_->texUnit = unit;
  } else {
    pimpl_->texUnit = GL_TEXTURE0 + unit;
  }
}

RENITY_API void GL_Texture2D::use() {
  glActiveTexture(pimpl_->texUnit);
  glBindTexture(GL_TEXTURE_2D, pimpl_->tex);

  // TODO: Find out if this is really needed to bind >1 texture at a time
  // glUniform1i(glGetUniformLocation(shaderProgram, "myTexture"), 0);
}

RENITY_API Dimension2Du32 GL_Texture2D::getSize() const { return pimpl_->size; }
}  // namespace renity
