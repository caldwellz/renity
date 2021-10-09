/***************************************************
* Point2D.h: 2-dimensional point template          *
* Copyright (C) 2021 by Zach Caldwell              *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#ifndef RENITY_POINT2D_H_
#define RENITY_POINT2D_H_

#include <SDL2/SDL_rect.h>
#include "types.h"

namespace renity {

/** Expresses a 2-dimensional point. */
template <typename T>
class Point2D {
  public:
    /** Default constructor.
     * Initializes x and y to 0.
     */
    Point2D() {
        x_ = y_ = 0;
    }

    /** Parameterized constructor that sets x and y to the given values.
     * \param x The initial x coordinate.
     * \param y The initial y coordinate.
     */
    Point2D(const T &x, const T &y) {
        x_ = x;
        y_ = y;
    }

    ~Point2D() = default;

    /** Get the x coordinate.
     * \returns The current x coordinate value.
     */
    T x() const {
        return x_;
    }

    /** Set the x coordinate.
     * \param x The new x coordinate value.
     */
    void x(const T &new_x) {
        x_ = new_x;
    }

    /** Get the y coordinate.
     * \returns The current y coordinate value.
     */
    T y() const {
        return y_;
    }

    /** Set the y coordinate.
     * \param x The new y coordinate value.
     */
    void y(const T &new_y) {
        y_ = new_y;
    }

    /** Utility function that returns the current Point as an SDL_Point.
     * \todo Determine/perform more appropriate conversions.
     * \returns An SDL_Point representing the current state of the Point.
     */
    SDL_Point toSDLPoint() const {
        SDL_Point point;
        point.x = (int) this->x();
        point.y = (int) this->y();
        return point;
    }

    // TODO: Mathematical operators here

  private:
    T x_, y_;
};

using Point2Di = Point2D<int>;
using Point2Di16 = Point2D<int16_t>;
using Point2Di32 = Point2D<int32_t>;
using Point2Di64 = Point2D<int64_t>;
using Point2Du = Point2D<unsigned int>;
using Point2Du16 = Point2D<uint16_t>;
using Point2Du32 = Point2D<uint32_t>;
using Point2Du64 = Point2D<uint64_t>;
using Point2Df = Point2D<float>;
using Point2Dd = Point2D<double>;
}
#endif // RENITY_POINT2D_H_
