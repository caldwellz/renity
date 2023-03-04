/****************************************************
 * Test - Image surface/texture utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "utils/surface_utils.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <assert.h>
#include <physfs.h>

#include "resources/hood_png_zip.h"

// Legacy function that was combined/removed in SDL3
SDL_Surface *SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height,
                                            int depth, Uint32 format) {
  return SDL_CreateSurface(width, height, format);
}

int main(int argc, char *argv[]) {
  // Initialization and basic sanity checks
  PHYSFS_init(argv[0]);
  assert(PHYSFS_isInit());
  assert(PHYSFS_mountMemory(hoodArchive, hoodArchiveLen, NULL, hoodArchiveName,
                            NULL, 1));
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  SDL_Window *window = SDL_CreateWindow("Image test", 0, 0, 128, 128, 0);
  assert(window);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, 0);
  assert(renderer);
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Pure blue

  // Check LoadPhysSurface
  SDL_Surface *surface = RENITY_LoadPhysSurface(hoodFile);
  assert(surface);

  /*
   * The test file should be a 64x64 image with a pure red background
   * (color key), which is relevant to the next three tests.
   */
  SDL_Point position = {64, 63};  // One pixel outside the image
  const Uint8 bogusRed = 42;
  const Uint8 bogusGreen = 50;
  const Uint8 bogusBlue = 60;
  const Uint8 bogusAlpha = 70;
  const Uint32 bogusColor =
      SDL_MapRGBA(surface->format, bogusRed, bogusGreen, bogusBlue, bogusAlpha);
  Uint8 red, green, blue, alpha;
  Uint32 color = bogusColor;

  // Test out-of-bounds SetPixelNative/GetPixelNative
  assert(RENITY_SetPixelNative(surface, &position, 0xFFFFFFFF) == SDL_FALSE);
  assert(RENITY_GetPixelNative(surface, &position, &color) == SDL_FALSE);
  assert(color == bogusColor);

  // Test in-bounds SetPixelNative + EnableColorKey
  position.x = 63;  // Last pixel of the image
  color = 0;
  assert(RENITY_SetPixelNative(surface, &position, bogusColor));
  assert(RENITY_EnableColorKey(surface, &position) == 0);
  assert(SDL_GetSurfaceColorKey(surface, &color) == 0);
  assert(color == bogusColor);

  // Test GetPixelRGBA with the sample image
  position.x = 0;
  assert(RENITY_GetPixelRGBA(surface, &position, &red, &green, &blue, &alpha));
  assert(red == 255);
  assert(green == 0);
  assert(blue == 0);
  assert(alpha == 255);

  // Test SetPixelRGBA + GetPixelRGBA together
  position.y = 0;
  SDL_Surface *blankSurface =
      SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, SDL_PIXELFORMAT_RGBA32);
  assert(blankSurface);
  assert(SDL_FillSurfaceRect(
             blankSurface, NULL,
             SDL_MapRGBA(blankSurface->format, 127, 127, 127, 255)) == 0);
  assert(RENITY_SetPixelRGBA(blankSurface, &position, bogusRed, bogusGreen,
                             bogusBlue, bogusAlpha));
  assert(RENITY_GetPixelRGBA(blankSurface, &position, &red, &green, &blue,
                             &alpha));
  assert(red == bogusRed);
  assert(green == bogusGreen);
  assert(blue == bogusBlue);
  assert(alpha == bogusAlpha);
  SDL_DestroySurface(blankSurface);

  // Display test
  assert(RENITY_EnableColorKey(surface, &position) == 0);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FRect destRect = {32, 32, 64, 64};
  for (Uint8 i = 0; i < 2; ++i) {
    SDL_RenderClear(renderer);
    assert(SDL_RenderTexture(renderer, texture, NULL, &destRect) == 0);
    SDL_RenderPresent(renderer);
    SDL_Delay(750);
  }

  // Clean up
  SDL_DestroySurface(surface);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  PHYSFS_deinit();

  return 0;
}
