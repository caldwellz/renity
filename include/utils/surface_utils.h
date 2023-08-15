/****************************************************
 * surface_utils.h: SDL_Surface utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_UTILS_SURFACE_UTILS_H_
#define RENITY_UTILS_SURFACE_UTILS_H_

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_rwops.h>
#include <SDL3/SDL_surface.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/** Load an SDL surface from an SDL_RWops.
 * @param src An RWops already opened for reading.
 * @return An SDL_Surface containg the image data, or NULL on failure.
 */
RENITY_API SDL_Surface *RENITY_LoadPhysSurfaceRW(SDL_RWops *src);

/**
 * Load an SDL surface from a PhysFS image file.
 * @param fname Image filename to read, in platform-independent notation.
 * @return An SDL_Surface containg the image data, or NULL on failure.
 */
RENITY_API SDL_Surface *RENITY_LoadPhysSurface(const char *fname);

/**
 * Flip a Surface horizontally into a new Surface.
 * Maintains the same color format / component order.
 * @param surf The SDL_Surface to flip.
 * @param freeSurf Whether to destroy the original surface.
 * @returns A new, horizontally-flipped surface, or NULL on failure.
 */
RENITY_API SDL_Surface *RENITY_FlipSurfaceHorizontal(SDL_Surface *surf,
                                                     SDL_bool freeSurf);

/**
 * Flip a Surface vertically into a new Surface.
 * @param surf The SDL_Surface to flip.
 * @param freeSurf Whether to destroy the original surface.
 * @returns A new, vertically-flipped surface, or NULL on failure.
 */
RENITY_API SDL_Surface *RENITY_FlipSurfaceVertical(SDL_Surface *surf,
                                                   SDL_bool freeSurf);

/**
 * Rotate a Surface 180 degrees into a new Surface.
 * Same effect as flipping both horizontally & vertically.
 * @param surf The SDL_Surface to flip.
 * @param freeSurf Whether to destroy the original surface.
 * @returns A new, flipped surface, or NULL on failure.
 */
RENITY_API SDL_Surface *RENITY_RotateSurface180(SDL_Surface *surf,
                                                SDL_bool freeSurf);

/**
 * Enable a Surface's transparency color key using a 2D pixel position.
 * If you later need to disable the color key, just use SDL_SetColorKey
 * directly.
 * @param surf The SDL_Surface on which to set the color key (transparent pixel
 * value).
 * @param pos The (zero-indexed) position of the color key pixel.
 * @return 0 on success or a negative error code on failure (i.e. the return
 * value of SDL_SetColorKey).
 */
RENITY_API int RENITY_EnableColorKey(SDL_Surface *surf, const SDL_Point *pos);

/**
 * Get the color value of a Surface pixel at a certain position (in its native
 * format).
 * @param surf The SDL_Surface from which to get pixel data.
 * @param pos The (zero-indexed) position of the target pixel.
 * @param destColor A destination pointer to set to the native-format color
 * value. Automatically endianness-converted.
 * @return SDL_TRUE on success or SDL_FALSE on failure.
 */
RENITY_API int RENITY_GetPixelNative(SDL_Surface *surf, const SDL_Point *pos,
                                     Uint32 *destColor);

/**
 * Set the color value of a Surface pixel at a certain position (in its native
 * format).
 * @param surf The SDL_Surface on which to set pixel data.
 * @param pos The (zero-indexed) position of the target pixel.
 * @param color The native-format color value to set. Automatically
 * endianness-converted.
 * @return SDL_TRUE on success or SDL_FALSE on failure.
 */
RENITY_API int RENITY_SetPixelNative(SDL_Surface *surf, const SDL_Point *pos,
                                     Uint32 color);

/**
 * Get the RGBA values of a Surface pixel at a certain position.
 * @param surf The SDL_Surface from which to get pixel colors.
 * @param pos The (zero-indexed) position of the target pixel.
 * @param r Pointer to be filled in with the Red component.
 * @param g Pointer to be filled in with the Green component.
 * @param b Pointer to be filled in with the Blue component.
 * @param a Pointer to be filled in with the Alpha component. Will be set to
 * 0xff (100% opaque) if the pixel format doesn't support an alpha channel.
 * @return SDL_TRUE on success or SDL_FALSE on failure.
 */
RENITY_API int RENITY_GetPixelRGBA(SDL_Surface *surf, const SDL_Point *pos,
                                   Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);

/**
 * Set the RGBA values of a Surface pixel at a certain position.
 * @param surf The SDL_Surface from which to get pixel colors.
 * @param pos (zero-indexed) The position of the target pixel.
 * @param r The Red component.
 * @param g The Green component.
 * @param b The Blue component.
 * @param a The Alpha component. Silently ignored if the pixel format doesn't
 * support an alpha channel.
 * @return SDL_TRUE on success or SDL_FALSE on failure.
 */
RENITY_API int RENITY_SetPixelRGBA(SDL_Surface *surf, const SDL_Point *pos,
                                   Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  // RENITY_UTILS_SURFACE_UTILS_H_
