/****************************************************
 * Test - PRODUCT version / revision                *
 * Copyright (C) 2021-2023 Zach Caldwell            *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "version.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  // Verify pointers are not null
  assert(PRODUCT_NAME);
  assert(PRODUCT_VERSION_STR);
  assert(PRODUCT_REVISION);
  assert(PRODUCT_BUILD_TYPE);
  assert(PRODUCT_COMPILER);
  assert(PRODUCT_COMPILER_VERSION);

  // Verify contents not null
  assert(strlen(PRODUCT_NAME));
  assert(strlen(PRODUCT_VERSION_STR));
  assert(strlen(PRODUCT_REVISION));
  assert(strlen(PRODUCT_BUILD_TYPE));
  assert(strlen(PRODUCT_COMPILER));
  assert(strlen(PRODUCT_COMPILER_VERSION));

  // Log the version strings
  printf("%s %s-%s-%s (%s-%s)\n", PRODUCT_NAME, PRODUCT_VERSION_STR,
         PRODUCT_REVISION, PRODUCT_BUILD_TYPE, PRODUCT_COMPILER,
         PRODUCT_COMPILER_VERSION);

  return 0;
}
