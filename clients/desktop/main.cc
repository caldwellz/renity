/****************************************************
 * Desktop client entry point                       *
 * Copyright (C) 2023 Zach Caldwell                 *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
  const char *platformName = SDL_GetPlatform();
  SDL_Log("Displaying MessageBox for platform '%s'.\n", platformName);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error Title", platformName,
                           NULL);

  return 0;
}
