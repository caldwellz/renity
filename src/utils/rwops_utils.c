/****************************************************
 * rwops_utils.c: SDL_RWops utility functions       *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "utils/rwops_utils.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "utils/physfsrwops.h"

RENITY_API Sint64 RENITY_WriteBufferToPath(const char* dest, const Uint8* src,
                                           Uint32 srcSize) {
  if (!src || !srcSize) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "RENITY_WriteBufferToPath: No data given to write (caller "
                 "specified %u bytes)\n",
                 srcSize);
    return 0;
  }

  SDL_RWops* destOps = PHYSFSRWOPS_openWrite(dest);
  if (!destOps) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                 "RENITY_WriteBufferToPath: Could not open '%s': %s\n", dest,
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return -1;
  }

  Sint64 writeCount = SDL_RWwrite(destOps, src, srcSize);
  SDL_DestroyRW(destOps);
  if (writeCount < srcSize) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                 "RENITY_WriteBufferToPath: Could not write complete buffer "
                 "(wrote %li out of %u bytes): %s\n",
                 writeCount, srcSize, SDL_GetError());
  }
  return writeCount;
}

RENITY_API Sint64 RENITY_ReadRawBufferMax(SDL_RWops* src, Uint8** bufOut,
                                          Uint32 maxSize) {
  if (src == NULL) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "RENITY_ReadRawBufferMax: src is NULL.\n");
    return -1;
  }

  Sint64 srcSize = SDL_RWsize(src);
  if (srcSize == 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "RENITY_ReadRawBufferMax: Stream size is 0.\n");
    return 0;
  }
  if (srcSize < 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "RENITY_ReadRawBufferMax: Could not determine stream size; "
                 "allocating buffer of max size (%#010x).\n",
                 maxSize);
    srcSize = maxSize;
  }

  Uint8* buf = (Uint8*)SDL_malloc(srcSize);
  Sint64 readBytes = SDL_RWread(src, buf, srcSize);
  SDL_DestroyRW(src);
  if (readBytes < 1) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "RENITY_ReadRawBufferMax: Could not read anything from "
                 "buffer (%li).\n",
                 readBytes);
    SDL_free(buf);
    return readBytes;
  }
  if (readBytes < srcSize) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "RENITY_ReadRawBufferMax: Could not read full buffer size (%li "
                "vs %li).\n",
                readBytes, srcSize);
  }

  *bufOut = buf;
  return readBytes;
}

RENITY_API Sint64 RENITY_ReadCharBufferMax(SDL_RWops* src, char** bufOut,
                                           Uint32 maxSize) {
  Uint8* buf = NULL;
  Sint64 readBytes = RENITY_ReadRawBufferMax(src, &buf, maxSize);
  if (readBytes >= 0) {
    SDL_realloc(buf, readBytes + 1);
    buf[readBytes] = '\0';
    *bufOut = (char*)buf;
  }

  return readBytes;
}
