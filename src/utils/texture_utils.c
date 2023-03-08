/***************************************************
 * texture_utils.h: SDL_Texture utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "utils/texture_utils.h"

#include "utils/surface_utils.h"

/** Load an SDL texture from a PhysFS image file. */
RENITY_API SDL_Texture *RENITY_LoadPhysTexture(SDL_Renderer *renderer,
                                               const char *fname) {
  return RENITY_LoadPhysTextureEx(renderer, fname, SDL_FALSE, 0);
}

/** Load an SDL texture from a PhysFS image file, with extended options. */
RENITY_API SDL_Texture *RENITY_LoadPhysTextureEx(SDL_Renderer *renderer,
                                                 const char *fname, int keyFlag,
                                                 const SDL_Point *keyPos) {
  if (renderer && fname) {
    SDL_Surface *surf = RENITY_LoadPhysSurface(fname);
    if (surf) {
      SDL_Texture *tex = NULL;
      if (keyFlag) {
        if (RENITY_EnableColorKey(surf, keyPos) != 0) return NULL;
      }
      tex = SDL_CreateTextureFromSurface(renderer, surf);
      SDL_DestroySurface(surf);
      return tex;
    }
  }

  return NULL;
}

/** Create an SDL texture from an SDL_RWops, with extended options. */
RENITY_API SDL_Texture *RENITY_LoadPhysTextureExRW(SDL_Renderer *renderer,
                                                   SDL_RWops *src, int keyFlag,
                                                   const SDL_Point *keyPos) {
  if (renderer && src) {
    SDL_Surface *surf = RENITY_LoadPhysSurfaceRW(src);
    if (surf) {
      SDL_Texture *tex =
          RENITY_CreateTextureFromSurfaceEx(renderer, surf, keyFlag, keyPos);
      SDL_DestroySurface(surf);
      return tex;
    }
  }

  return NULL;
}

/** Create an SDL texture from an SDL surface, with extended options. */
RENITY_API SDL_Texture *RENITY_CreateTextureFromSurfaceEx(
    SDL_Renderer *renderer, SDL_Surface *surf, int keyFlag,
    const SDL_Point *keyPos) {
  if (renderer && surf) {
    if (keyFlag) {
      if (RENITY_EnableColorKey(surf, keyPos) != 0) return NULL;
    }
    return SDL_CreateTextureFromSurface(renderer, surf);
  }

  return NULL;
}
