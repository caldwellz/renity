/****************************************************
 * Test - Image surface/texture utility functions   *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Texture.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <assert.h>
#include <physfs.h>

#include "Window.h"
#include "resources/epic_png_zip.h"

int main(int argc, char *argv[]) {
  // Initialization and basic sanity checks
  PHYSFS_init(argv[0]);
  assert(PHYSFS_isInit());
  assert(PHYSFS_mountMemory(epicArchive, epicArchiveLen, NULL, epicArchiveName,
                            NULL, 1));
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  renity::Window window;
  window.useFullscreen(false, false);
  window.size(renity::Dimension2Di(160, 160));
  assert(window.open() && window.isOpen());

  // Check constructors
  renity::Texture defTex;
  assert(!defTex.isValid());
  renity::Texture basicTex(window);
  assert(!basicTex.isValid());
  renity::Texture fullTex(window, epicFile);
  assert(fullTex.isValid());

  // Check uninitialized texture properties
  renity::Dimension2Di uninitializedSize = basicTex.getSize();
  assert(0 == uninitializedSize.width());
  assert(0 == uninitializedSize.height());
  assert(0 == basicTex.getImagePath().compare(""));

  // Check initialized image properties
  renity::Dimension2Di epicSize = fullTex.getSize();
  assert(epicWidth == epicSize.width());
  assert(epicHeight == epicSize.height());
  assert(0 == fullTex.getImagePath().compare(epicFile));

  // Check unload/reload
  assert(fullTex.isValid());
  fullTex.unload();
  assert(!fullTex.isValid());
  assert(fullTex.load(epicFile));
  assert(fullTex.isValid());
  assert(fullTex.load(epicFile));
  assert(fullTex.isValid());
  fullTex.unload();
  assert(!fullTex.isValid());

  // Check window-setting
  assert(defTex.setWindow(window));
  assert(defTex.load(epicFile));
  assert(defTex.isValid());

  // Check color key
  renity::Point2Di colorKeyPos(1, 1);
  assert(!defTex.isColorKeyEnabled());
  assert(defTex.enableColorKey(colorKeyPos));
  assert(defTex.isColorKeyEnabled());
  assert(defTex.isValid());
  defTex.disableColorKey();
  assert(!defTex.isColorKeyEnabled());
  assert(defTex.isValid());
  assert(defTex.enableColorKey(colorKeyPos));

  // Display test
  renity::Rect2Di destTop(41, 0, epicWidth, epicHeight);
  renity::Rect2Di destBottom(41, epicHeight + 2, epicWidth, epicHeight);
  for (Uint8 i = 0; i < 2; ++i) {
    assert(defTex.draw(nullptr, &destTop));
    assert(defTex.draw(nullptr, &destBottom, 90, nullptr, true, true));
    SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 255,
                           SDL_ALPHA_OPAQUE);  // Pure blue
    window.update();
    SDL_Delay(750);
  }

  // Clean up
  defTex.unload();
  window.close();
  SDL_Quit();
  PHYSFS_deinit();

  return 0;
}
