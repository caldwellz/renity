/***************************************************
* Test - Point2D                                   *
* Copyright (C) 2021 Zach Caldwell                 *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "Point2D.h"

#include <assert.h>

int main(void) {
    // Default constructor
    renity::Point2Du pointDefault;
    assert(0 == pointDefault.x());
    assert(0 == pointDefault.y());

    // Parameterized constructor
    renity::Point2Du pointParams(2, 3.7f);
    assert(2 == pointParams.x());
    assert(3 == pointParams.y());

    // Copy constructor
    renity::Point2Du pointCopy(pointParams);
    assert(2 == pointCopy.x());
    assert(3 == pointCopy.y());

    // Assignment operator
    renity::Point2Du pointAssign;
    pointAssign = pointCopy;
    assert(2 == pointAssign.x());
    assert(3 == pointAssign.y());

    // Setters & Getters
    pointDefault.x(3.7f);
    pointDefault.y(5);
    assert(3 == pointDefault.x());
    assert(5 == pointDefault.y());

    /* TODO: Mathematical operators
    renity::Point2Du pointSubtract = pointDefault - pointAssign;
    assert(1 == pointSubtract.x());
    assert(2 == pointSubtract.y());
    */

    return 0;
}
