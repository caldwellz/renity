/***************************************************
* Test - Rect2D                                    *
* Copyright (C) 2021 Zach Caldwell                 *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "Rect2D.h"

#include <assert.h>

int main(void) {
    // Default constructor
    renity::Rect2Du rectDefault;
    assert(0 == rectDefault.x());
    assert(0 == rectDefault.y());
    assert(0 == rectDefault.width());
    assert(0 == rectDefault.height());

    // Parameterized constructor
    renity::Rect2Du rectParams(2, 3.7f, 4, 5.33f);
    assert(2 == rectParams.x());
    assert(3 == rectParams.y());
    assert(4 == rectParams.width());
    assert(5 == rectParams.height());

    // Copy constructor
    renity::Rect2Du rectCopy(rectParams);
    assert(2 == rectCopy.x());
    assert(3 == rectCopy.y());
    assert(4 == rectParams.width());
    assert(5 == rectParams.height());

    // Assignment operator
    renity::Rect2Du rectAssign;
    rectAssign = rectCopy;
    assert(2 == rectAssign.x());
    assert(3 == rectAssign.y());
    assert(4 == rectParams.width());
    assert(5 == rectParams.height());

    // Setters & Getters
    rectDefault.x(4.7f);
    rectDefault.y(5);
    rectDefault.width(6.7f);
    rectDefault.height(7);
    assert(4 == rectDefault.x());
    assert(5 == rectDefault.y());
    assert(6 == rectDefault.width());
    assert(7 == rectDefault.height());

    // SDL_Rect export
    SDL_Rect rectSDL = rectDefault.toSDLRect();
    assert(4 == rectSDL.x);
    assert(5 == rectSDL.y);
    assert(6 == rectSDL.w);
    assert(7 == rectSDL.h);

    // Mathematical functions
    assert(42 == rectDefault.getArea());

    /* TODO: Mathematical operators
    renity::Rect2Du rectSubtract = rectDefault - rectAssign;
    assert(2 == rectSubtract.width());
    assert(2 == rectSubtract.height());
    */

    return 0;
}
