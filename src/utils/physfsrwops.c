/*
 * This code provides a glue layer between PhysicsFS and Simple Directmedia
 *  Layer's (SDL) RWops i/o abstraction.
 *
 * License: this code is public domain. I make no warranty that it is useful,
 *  correct, harmless, or environmentally safe.
 *
 * This particular file may be used however you like, including copying it
 *  verbatim into a closed-source project, exploiting it commercially, and
 *  removing any trace of my name from the source (although I hope you won't
 *  do that). I welcome enhancements and corrections to this file, but I do
 *  not require you to send me patches if you make changes. This code has
 *  NO WARRANTY.
 *
 * Unless otherwise stated, the rest of PhysicsFS falls under the zlib license.
 *  Please see LICENSE.txt in the root of the source tree.
 *
 * SDL 1.2 falls under the LGPL license. SDL 1.3+ is zlib, like PhysicsFS.
 *  You can get SDL at https://www.libsdl.org/
 *
 *  This file was written by Ryan C. Gordon. (icculus@icculus.org).
 *  Modifications made by Zach Caldwell to the declspecs, includes, and to
 * remove SDL1.x support.
 */

#include "utils/physfsrwops.h"

#include <stdio.h> /* used for SEEK_SET, SEEK_CUR, SEEK_END ... */

/* SDL's RWOPS interface changed a little in SDL 3.0... */
#if defined(SDL_VERSION_ATLEAST)
#if SDL_VERSION_ATLEAST(3, 0, 0)
#define TARGET_SDL3 1
#endif
#endif

#if !TARGET_SDL3
#error \
    "Only SDL3 is supported by this library. Please download original PhysFS files if you need SDL 1.x support."
#endif

static Sint64 physfsrwops_size(struct SDL_RWops *rw) {
  PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
  return (Sint64)PHYSFS_fileLength(handle);
} /* physfsrwops_size */

static Sint64 physfsrwops_seek(struct SDL_RWops *rw, Sint64 offset,
                               int whence) {
  PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
  PHYSFS_sint64 pos = 0;

  if (whence == SDL_RW_SEEK_SET)
    pos = (PHYSFS_sint64)offset;

  else if (whence == SDL_RW_SEEK_CUR) {
    const PHYSFS_sint64 current = PHYSFS_tell(handle);
    if (current == -1) {
      SDL_SetError("Can't find position in file: %s",
                   PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
      return -1;
    } /* if */

    if (offset == 0) /* this is a "tell" call. We're done. */
    {
      return (Sint64)current;
    } /* if */

    pos = current + ((PHYSFS_sint64)offset);
  } /* else if */

  else if (whence == SDL_RW_SEEK_END) {
    const PHYSFS_sint64 len = PHYSFS_fileLength(handle);
    if (len == -1) {
      SDL_SetError("Can't find end of file: %s",
                   PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
      return -1;
    } /* if */

    pos = len + ((PHYSFS_sint64)offset);
  } /* else if */

  else {
    SDL_SetError("Invalid 'whence' parameter.");
    return -1;
  } /* else */

  if (pos < 0) {
    SDL_SetError("Attempt to seek past start of file.");
    return -1;
  } /* if */

  if (!PHYSFS_seek(handle, (PHYSFS_uint64)pos)) {
    SDL_SetError("PhysicsFS error: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return -1;
  } /* if */

  return (Sint64)pos;
} /* physfsrwops_seek */

static Sint64 physfsrwops_read(struct SDL_RWops *rw, void *ptr, Sint64 size) {
  PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
  const PHYSFS_uint64 readlen = (PHYSFS_uint64)(size);
  const PHYSFS_sint64 rc = PHYSFS_readBytes(handle, ptr, readlen);
  if (rc != ((PHYSFS_sint64)readlen)) {
    if (!PHYSFS_eof(handle)) /* not EOF? Must be an error. */
    {
      SDL_SetError("PhysicsFS error: %s",
                   PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

      return 0;
    } /* if */
  }   /* if */

  return rc;
} /* physfsrwops_read */

static Sint64 physfsrwops_write(struct SDL_RWops *rw, const void *ptr,
                                Sint64 size) {
  PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
  const PHYSFS_uint64 writelen = (PHYSFS_uint64)(size);
  const PHYSFS_sint64 rc = PHYSFS_writeBytes(handle, ptr, writelen);
  if (rc != ((PHYSFS_sint64)writelen))
    SDL_SetError("PhysicsFS error: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

  return rc;
} /* physfsrwops_write */

static int physfsrwops_close(SDL_RWops *rw) {
  PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
  if (!PHYSFS_close(handle)) {
    SDL_SetError("PhysicsFS error: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return -1;
  } /* if */

  SDL_DestroyRW(rw);
  return 0;
} /* physfsrwops_close */

static SDL_RWops *create_rwops(PHYSFS_File *handle) {
  SDL_RWops *retval = NULL;

  if (handle == NULL)
    SDL_SetError("PhysicsFS error: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  else {
    retval = SDL_CreateRW();
    if (retval != NULL) {
      retval->size = physfsrwops_size;
      retval->seek = physfsrwops_seek;
      retval->read = physfsrwops_read;
      retval->write = physfsrwops_write;
      retval->close = physfsrwops_close;
      retval->hidden.unknown.data1 = handle;
    } /* if */
  }   /* else */

  return retval;
} /* create_rwops */

RENITY_API SDL_RWops *PHYSFSRWOPS_makeRWops(PHYSFS_File *handle) {
  SDL_RWops *retval = NULL;
  if (handle == NULL)
    SDL_SetError("NULL pointer passed to PHYSFSRWOPS_makeRWops().");
  else
    retval = create_rwops(handle);

  return retval;
} /* PHYSFSRWOPS_makeRWops */

RENITY_API SDL_RWops *PHYSFSRWOPS_openRead(const char *fname) {
  return create_rwops(PHYSFS_openRead(fname));
} /* PHYSFSRWOPS_openRead */

RENITY_API SDL_RWops *PHYSFSRWOPS_openWrite(const char *fname) {
  return create_rwops(PHYSFS_openWrite(fname));
} /* PHYSFSRWOPS_openWrite */

RENITY_API SDL_RWops *PHYSFSRWOPS_openAppend(const char *fname) {
  return create_rwops(PHYSFS_openAppend(fname));
} /* PHYSFSRWOPS_openAppend */

/* end of physfsrwops.c ... */
