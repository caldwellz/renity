/****************************************************
 * texture_utils.h: SDL_Texture utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_UTILS_TEXTURE_UTILS_H_
#define RENITY_UTILS_TEXTURE_UTILS_H_

#include <SDL3/SDL_render.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * Load an SDL texture from a PhysFS image file.
 * @param renderer The rendering context.
 * @param fname Image filename to read, in platform-independent notation.
 * @return An SDL_Texture containg the image data, or NULL on failure.
 */
RENITY_API SDL_Texture *RENITY_LoadPhysTexture(SDL_Renderer *renderer,
                                               const char *fname);

/**
 * Load an SDL texture from a PhysFS image file, with extended
 * options applied to the surface before converting to a texture.
 * Currently adds the ability to set a color key.
 * @param renderer The rendering context.
 * @param fname Image filename to read, in platform-independent notation.
 * @param keyFlag Flag indicating whether to enable a transparency color key.
 * @param keyPos The (zero-indexed) position of the color key pixel.
 * @return An SDL_Texture containg the image data, or NULL on failure.
 */
RENITY_API SDL_Texture *RENITY_LoadPhysTextureEx(SDL_Renderer *renderer,
                                                 const char *fname, int keyFlag,
                                                 const SDL_Point *keyPos);

#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  // RENITY_UTILS_TEXTURE_UTILS_H_
