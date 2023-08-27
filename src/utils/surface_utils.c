/****************************************************
 * surface_utils.h: SDL_Surface utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "utils/surface_utils.h"

#include <SDL3/SDL_image.h>
#include <SDL3/SDL_stdinc.h>

#include "utils/physfsrwops.h"

/** Load an SDL surface from an SDL_RWops. */
RENITY_API SDL_Surface *RENITY_LoadPhysSurfaceRW(SDL_RWops *src) {
  if (src) {
    return IMG_Load_RW(src, SDL_TRUE);
  }
  return NULL;
}

/** Load an SDL surface from a PhysFS image file. */
RENITY_API SDL_Surface *RENITY_LoadPhysSurface(const char *fname) {
  if (fname) {
    SDL_RWops *rw = PHYSFSRWOPS_openRead(fname);
    return RENITY_LoadPhysSurfaceRW(rw);
  }

  return NULL;
}

RENITY_API SDL_Surface *RENITY_FlipSurfaceHorizontal(SDL_Surface *surf,
                                                     SDL_bool freeSurf) {
  if (!surf) return NULL;
  SDL_Point pt;
  SDL_Surface *flippedSurf =
      SDL_CreateSurface(surf->w, surf->h, surf->format->format);
  SDL_LockSurface(flippedSurf);
  SDL_LockSurface(surf);

  for (pt.y = 0; pt.y < surf->h; ++pt.y) {
    for (pt.x = 0; pt.x < surf->w; ++pt.x) {
      Uint32 color;
      SDL_Point srcPt = pt;
      srcPt.x = surf->w - pt.x - 1;
      RENITY_GetPixelNative(surf, &srcPt, &color);
      RENITY_SetPixelNative(flippedSurf, &pt, color);
    }
  }
  SDL_UnlockSurface(surf);
  SDL_UnlockSurface(flippedSurf);
  if (freeSurf) {
    SDL_DestroySurface(surf);
  }
  return flippedSurf;
}

RENITY_API SDL_Surface *RENITY_FlipSurfaceVertical(SDL_Surface *surf,
                                                   SDL_bool freeSurf) {
  if (!surf) return NULL;
  SDL_Surface *flippedSurf =
      SDL_CreateSurface(surf->w, surf->h, surf->format->format);
  SDL_LockSurface(flippedSurf);
  SDL_LockSurface(surf);
  for (int rowCount = 0; rowCount < surf->h; ++rowCount) {
    Uint8 *destRow =
        (Uint8 *)flippedSurf->pixels + (rowCount * flippedSurf->pitch);
    Uint8 *srcRow =
        (Uint8 *)surf->pixels + ((surf->h - rowCount - 1) * surf->pitch);
    SDL_memcpy(destRow, srcRow, surf->pitch);
  }
  SDL_UnlockSurface(surf);
  SDL_UnlockSurface(flippedSurf);
  if (freeSurf) {
    SDL_DestroySurface(surf);
  }
  return flippedSurf;
}

RENITY_API SDL_Surface *RENITY_RotateSurface180(SDL_Surface *surf,
                                                SDL_bool freeSurf) {
  if (!surf) return NULL;
  SDL_Point srcPt, destPt;
  SDL_Surface *flippedSurf =
      SDL_CreateSurface(surf->w, surf->h, surf->format->format);
  SDL_LockSurface(flippedSurf);
  SDL_LockSurface(surf);
  for (srcPt.x = 0; srcPt.x < surf->w; ++srcPt.x) {
    for (srcPt.y = 0; srcPt.y < surf->h; ++srcPt.y) {
      Uint32 color;
      destPt.x = surf->w - srcPt.x - 1;
      destPt.y = surf->h - srcPt.y - 1;
      RENITY_GetPixelNative(surf, &srcPt, &color);
      RENITY_SetPixelNative(flippedSurf, &destPt, color);
    }
  }
  SDL_UnlockSurface(surf);
  SDL_UnlockSurface(flippedSurf);
  if (freeSurf) {
    SDL_DestroySurface(surf);
  }
  return flippedSurf;
}

/** Enable a Surface's transparency color key using a pixel position. */
RENITY_API int RENITY_EnableColorKey(SDL_Surface *surf, const SDL_Point *pos) {
  Uint32 color;
  if (RENITY_GetPixelNative(surf, pos, &color))
    return SDL_SetSurfaceColorKey(surf, SDL_TRUE, color);

  return -1;
}

// Helper function to convert a position point to a byte-wide pixel pointer
// Does not lock/unlock the Surface
static inline Uint8 *getPixelPointer(SDL_Surface *surf, const SDL_Point *pos) {
  if (surf && pos && (pos->x >= 0) && (pos->y >= 0)) {
    Uint32 surfSize = surf->h * surf->pitch;
    Uint32 offset =
        (pos->y * surf->pitch) + (pos->x * surf->format->BytesPerPixel);
    if (offset < surfSize) return (Uint8 *)surf->pixels + offset;
  }

  return NULL;
}

/** Get the color value of a Surface pixel at a certain position (in its native
 * format). */
RENITY_API int RENITY_GetPixelNative(SDL_Surface *surf, const SDL_Point *pos,
                                     Uint32 *destColor) {
  if (destColor) {
    SDL_LockSurface(surf);
    Uint8 *pixelPtr = getPixelPointer(surf, pos);
    if (pixelPtr) {
      // From http://sdl.beuc.net/sdl.wiki/Pixel_Access
      switch (surf->format->BytesPerPixel) {
        case 1:
          *destColor = *pixelPtr;
          break;
        case 2:
          *destColor = *(Uint16 *)pixelPtr;
          break;
        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
          *destColor = pixelPtr[0] << 16 | pixelPtr[1] << 8 | pixelPtr[2];
#else
          *destColor = pixelPtr[0] | pixelPtr[1] << 8 | pixelPtr[2] << 16;
#endif
          break;
        case 4:
          *destColor = *(Uint32 *)pixelPtr;
          break;
        default:
          SDL_UnlockSurface(surf);
          return SDL_FALSE;
      }
      SDL_UnlockSurface(surf);
      return SDL_TRUE;
    }
    SDL_UnlockSurface(surf);
    return SDL_FALSE;
  }

  return SDL_FALSE;
}

/** Set the color value of a Surface pixel at a certain position (in its native
 * format). */
RENITY_API int RENITY_SetPixelNative(SDL_Surface *surf, const SDL_Point *pos,
                                     Uint32 color) {
  SDL_LockSurface(surf);
  Uint8 *pixelPtr = getPixelPointer(surf, pos);
  if (pixelPtr) {
    // From http://sdl.beuc.net/sdl.wiki/Pixel_Access
    switch (surf->format->BytesPerPixel) {
      case 1:
        *pixelPtr = (Uint8)color;
        break;
      case 2:
        *(Uint16 *)pixelPtr = (Uint16)color;
        break;
      case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        pixelPtr[0] = (color >> 16) & 0xff;
        pixelPtr[1] = (color >> 8) & 0xff;
        pixelPtr[2] = color & 0xff;
#else
        pixelPtr[0] = color & 0xff;
        pixelPtr[1] = (color >> 8) & 0xff;
        pixelPtr[2] = (color >> 16) & 0xff;
#endif
        break;
      case 4:
        *(Uint32 *)pixelPtr = color;
        break;
      default:
        SDL_UnlockSurface(surf);
        return SDL_FALSE;
    }
    SDL_UnlockSurface(surf);
    return SDL_TRUE;
  }
  SDL_UnlockSurface(surf);
  return SDL_FALSE;
}

/** Get the RGBA values of a Surface pixel at a certain position. */
RENITY_API int RENITY_GetPixelRGBA(SDL_Surface *surf, const SDL_Point *pos,
                                   Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
  Uint32 nativeColor;
  if (r && g && b && a && RENITY_GetPixelNative(surf, pos, &nativeColor)) {
    SDL_GetRGBA(nativeColor, surf->format, r, g, b, a);
    return SDL_TRUE;
  }

  return SDL_FALSE;
}

/** Set the RGBA values of a Surface pixel at a certain position. */
RENITY_API int RENITY_SetPixelRGBA(SDL_Surface *surf, const SDL_Point *pos,
                                   Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  if (surf) {
    Uint32 nativeColor = SDL_MapRGBA(surf->format, r, g, b, a);
    return RENITY_SetPixelNative(surf, pos, nativeColor);
  }

  return SDL_FALSE;
}
