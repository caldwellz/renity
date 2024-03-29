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

#include "Application.h"
#include "types.h"

int main(int argc, char *argv[]) {
  renity::Application app(argc, argv);
  if (!app.initialize(false)) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                    "Could not initialize application! Last SDL error: %s\n",
                    SDL_GetError());
    renity::String boxMsg(
        "Could not initialize application!\n"
        "Please check logs for further details.\n"
        "Last error: ");
    boxMsg += SDL_GetError();
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", boxMsg.c_str(),
                             NULL);
    return 1;
  }
  return app.run();
}
