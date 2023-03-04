/****************************************************
 * Test - Window management class                   *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Window.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <assert.h>
#include <stdio.h>

#include "types.h"

int main(int argc, char *argv[]) {
  // Initialization and basic sanity checks
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  renity::Window window;
  window.useFullscreen(false, false);
  printf("- Window: Opening 1st window\n");
  assert(window.open() && window.isOpen());
  assert(window.update());
  assert(!window.isFullscreen());

  // Title
  renity::String new_title("Renity Window Test - New Title");
  printf("- Window: Changing title\n");
  window.title(new_title);
  assert(window.title() == new_title);

  // Position
  printf("- Window: Setting position to (1, 10)\n");
  renity::Point2Di32 pos(1, 10);
  window.position(pos);
  pos = window.position();
  // The window manager may adjust the position due to window decorations
  printf("Position after repositioning: (%i, %i)\n", pos.x(), pos.y());
  assert(pos.x() >= 1 && pos.y() >= 10);
  printf("- Window: Centering position\n");
  window.centerPosition();
  pos = window.position();
  printf("Position after centering: (%i, %i)\n", pos.x(), pos.y());
  assert(pos.x() && pos.y());

  // Windowed-mode size
  printf("- Window: Changing windowed-mode size to 800x600\n");
  renity::Dimension2Di size = window.size();
  printf("Size before change: %ix%i\n", size.width(), size.height());
  assert(size.width() && size.height());  // Dimensions should never be zero
  size.width(800);
  size.height(600);
  assert(window.size(size));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 800 && size.height() == 600);
  size = window.sizeInPixels();
  // HighDPI devices will have more pixels in the same screen coordinates
  printf("PixelSize after change: %ix%i\n", size.width(), size.height());
  assert(size.width() >= 800 && size.height() >= 600);

  // Changing to fullscreen - existing resolution
  /*
  printf("- Window: Changing to fullscreen, reusing existing resolution\n");
  assert(window.useFullscreen(true, false) && window.isFullscreen());
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 800 && size.height() == 600);
  */
  // Changing resolution while in an existing custom fullscreen video mode
  printf("- Window: Changing fullscreen resolution to 640x480\n");
  size.width(640);
  size.height(480);
  assert(window.size(size));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 640 && size.height() == 480);

  // Changing back to windowed mode
  printf("- Window: Changing back to windowed mode\n");
  assert(window.useFullscreen(false, false));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 640 && size.height() == 480);

  // Going fullscreen, native screen resolution
  /*
  printf("- Window: Changing to fullscreen, native screen resolution\n");
  assert(window.useFullscreen(true, true));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() && size.height());
  */
  // Changing resolution while in native fullscreen

  printf("- Window: Changing fullscreen resolution to 640x480\n");
  size.width(640);
  size.height(480);
  assert(window.size(size));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 640 && size.height() == 480);

  // Changing back to windowed mode again
  printf("- Window: Changing back to windowed mode again\n");
  assert(window.useFullscreen(false, false));
  assert(window.update());
  size = window.size();
  printf("Size after change: %ix%i\n", size.width(), size.height());
  assert(size.width() == 640 && size.height() == 480);

  // Create a second window
  printf("- Window: Opening 2nd window\n");
  renity::Window window2;
  window2.useFullscreen(false, false);
  assert(window2.open() && window2.isOpen());
  assert(window2.update());
  assert(!window2.isFullscreen());

  // Activate and update each window
  assert(window.activate());
  assert(window.update());
  assert(window2.activate());
  assert(window2.update());

  // Clean up
  window.close();
  window2.close();
  SDL_Quit();

  return 0;
}
