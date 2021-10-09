/***************************************************
* Rect2D.h: Rectangle template                     *
* Copyright (C) 2021 by Zach Caldwell              *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#ifndef RENITY_RECT2D_H_
#define RENITY_RECT2D_H_

#include "Dimension2D.h"
#include "Point2D.h"

#include <SDL2/SDL_rect.h>

namespace renity {

/** Expresses a rectangle containing both a position and set of dimensions. */
template <typename T>
class Rect2D: public Dimension2D<T>, public Point2D<T> {
  public:
    /** Default constructor */
    Rect2D() = default;

    /** Parameterized constructor that inisitalizes the Rect with the given values.
     * \param x The initial x coordinate.
     * \param y The initial y coordinate.
     * \param width The initial width.
     * \param height The initial height.
     */
    Rect2D(const T &x, const T &y, const T &width, const T &height)
    : Dimension2D<T>(width, height)
    , Point2D<T>(x, y)
    {}

    /** Utility function that returns the current rectangle as an SDL_Rect.
     * \todo Determine/perform more appropriate conversions.
     * \returns An SDL_Rect representing the current state of the rectangle.
     */
    SDL_Rect toSDLRect() const {
        SDL_Rect rect;
        rect.x = (int) this->x();
        rect.y = (int) this->y();
        rect.w = (int) this->width();
        rect.h = (int) this->height();
        return rect;
    }

    /** Default destructor. */
    ~Rect2D() = default;
};

using Rect2Di = Rect2D<int>;
using Rect2Di16 = Rect2D<int16_t>;
using Rect2Di32 = Rect2D<int32_t>;
using Rect2Di64 = Rect2D<int64_t>;
using Rect2Du = Rect2D<unsigned int>;
using Rect2Du16 = Rect2D<uint16_t>;
using Rect2Du32 = Rect2D<uint32_t>;
using Rect2Du64 = Rect2D<uint64_t>;
using Rect2Df = Rect2D<float>;
using Rect2Dd = Rect2D<double>;
}
#endif // RENITY_RECT2D_H_
