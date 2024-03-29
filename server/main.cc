/****************************************************
 * Headless server entry point                      *
 * Copyright (C) 2023 Zach Caldwell                 *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Application.h"
#include "version.h"
using namespace renity;

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Log the version and publisher strings to the console
  printf("%s %s.%i-%s-%s (%s-%s)\n", PRODUCT_NAME, PRODUCT_VERSION_STR,
         PRODUCT_VERSION_BUILD, PRODUCT_BUILD_TYPE, PRODUCT_REVISION,
         PRODUCT_COMPILER, PRODUCT_COMPILER_VERSION);
  if (strlen(PRODUCT_DESCRIPTION) > 0) {
    printf("%s\n", PRODUCT_DESCRIPTION);
  }
  if (strlen(PUBLISHER_NAME) > 0) {
    printf("Published by %s\n", PUBLISHER_NAME);
  }

  // Start headless app
  renity::Application app(argc, argv);
  if (!app.initialize(true)) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                    "Could not initialize application! Last SDL error: %s\n",
                    SDL_GetError());
    return 1;
  }
  int status = app.run();
  app.destroy();
  return status;
}
