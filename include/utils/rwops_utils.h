/****************************************************
 * rwops_utils.h: SDL_RWops utility functions       *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_UTILS_RWOPS_UTILS_H_
#define RENITY_UTILS_RWOPS_UTILS_H_

#include <SDL3/SDL_rwops.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/** Write a Uint8 buffer to a PhysFS path.
 * @param dest PhysFS file path under the active write dir to output to.
 * @param src A Uint8 buffer.
 * @param srcSize Size of the buffer.
 * @return The number of bytes written (should match srcSize), or -1 on failure.
 */
RENITY_API Sint64 RENITY_WriteBufferToPath(const char* dest, const Uint8* src,
                                           Uint32 srcSize);

/** Fill an unterminated Uint8 buffer from an SDL_RWops.
 * @param src An RWops already opened for reading. Closes it when finished.
 * @param bufOut A pointer to fill in with the destination buffer. Left
 * unchanged on failure or if no bytes were read.
 * @param maxSize Max bytes to read.
 * @return The number of bytes read, or -1 on failure.
 */
RENITY_API Sint64 RENITY_ReadRawBufferMax(SDL_RWops* src, Uint8** bufOut,
                                          Uint32 maxSize);

/** Fill an unterminated Uint8 buffer from an SDL_RWops, up to 16MB.
 * @param src An RWops already opened for reading. Closes it when finished.
 * @param bufOut A pointer to fill in with the destination buffer.
 * Left unchanged on failure or if no bytes were read.
 * @return The number of bytes read, or -1 on failure.
 */
#define RENITY_ReadRawBuffer(src, bufOut) \
  RENITY_ReadRawBufferMax(src, bufOut, 1 << 24)

/** Fill a null-terminated char buffer from an SDL_RWops.
 * @param src An RWops already opened for reading. Closes it when finished.
 * @param bufOut A pointer to fill in with the destination char buffer. Left
 * unchanged on failure or if no bytes were read.
 * @param maxSize Max bytes to read.
 * @return The number of bytes read, or -1 on failure.
 */
RENITY_API Sint64 RENITY_ReadCharBufferMax(SDL_RWops* src, char** bufOut,
                                           Uint32 maxSize);

/** Fill a null-terminated char buffer from an SDL_RWops, up to 16MB.
 * @param src An RWops already opened for reading. Closes it when finished.
 * @param bufOut A pointer to fill in with the destination char buffer.
 * Left unchanged on failure, or a ptr to a single null if no bytes were read.
 * @return The number of bytes read (not including null terminator), or -1 on
 * failure.
 */
#define RENITY_ReadCharBuffer(src, bufOut) \
  RENITY_ReadCharBufferMax(src, bufOut, 1 << 24)

#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  // RENITY_UTILS_RWOPS_UTILS_H_
