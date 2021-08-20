/***************************************************
* Test - Dimension2D                               *
* Copyright (C) 2021 Zach Caldwell                 *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "Dimension2D.h"

#include <assert.h>

int main(void) {
    // Default constructor
    renity::Dimension2Du dimDefault;
    assert(0 == dimDefault.width());
    assert(0 == dimDefault.height());

    // Parameterized constructor
    renity::Dimension2Du dimParams(2, 3.7f);
    assert(2 == dimParams.width());
    assert(3 == dimParams.height());

    // Copy constructor
    renity::Dimension2Du dimCopy(dimParams);
    assert(2 == dimCopy.width());
    assert(3 == dimCopy.height());

    // Assignment operator
    renity::Dimension2Du dimAssign;
    dimAssign = dimCopy;
    assert(2 == dimAssign.width());
    assert(3 == dimAssign.height());

    // Setters & Getters
    dimDefault.width(4.7f);
    dimDefault.height(5);
    assert(4 == dimDefault.width());
    assert(5 == dimDefault.height());

    // Mathematical functions
    assert(20 == dimDefault.getArea());

    return 0;
}
