/****************************************************
 * Headless server entry point                      *
 * Copyright (C) 2023 Zach Caldwell                 *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "version.h"
using namespace renity;

#include <SDL3/SDL_main.h>
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

  return 0;
}
