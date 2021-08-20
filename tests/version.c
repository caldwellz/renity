/***************************************************
* Test - Project version / revision                *
* Copyright (C) 2021 Zach Caldwell                 *
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
    assert(kProjectName);
    assert(kProjectVersion);
    assert(kProjectRevision);
    assert(kProjectCompiler);
    assert(kProjectCompilerVersion);

    // Verify contents are not empty
    assert(strlen(kProjectName));
    assert(strlen(kProjectVersion));
    assert(strlen(kProjectRevision));
    assert(strlen(kProjectCompiler));
    assert(strlen(kProjectCompilerVersion));

    // Log the version strings
    printf("%s %s-%s (%s-%s)\n", kProjectName, kProjectVersion, kProjectRevision, kProjectCompiler, kProjectCompilerVersion);

    return 0;
}
