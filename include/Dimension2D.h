/***************************************************
* Dimension2D.h: 2-dimensional size template       *
* Copyright (C) 2021 by Zach Caldwell              *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#ifndef RENITY_DIMENSION2D_H_
#define RENITY_DIMENSION2D_H_

#include "types.h"

namespace renity {

/** Expresses a 2-dimensional size. */
template <typename T>
class Dimension2D {
  public:
    /** Default constructor.
     * Initializes width and height to 0.
     */
    Dimension2D() {
        width_ = height_ = 0;
    }

    /** Parameterized constructor that sets width and height to the given values.
     * \param width The initial width.
     * \param height The initial height.
     */
    Dimension2D(const T &width, const T &height) {
        width_ = width;
        height_ = height;
    }

    ~Dimension2D() = default;

    /** Get the width.
     * \returns The current width value.
     */
    T width() const {
        return width_;
    }

    /** Set the width.
     * \param width The new width value.
     */
    void width(const T &new_width) {
        width_ = new_width;
    }

    /** Get the height.
     * \returns The current height value.
     */
    T height() const {
        return height_;
    }

    /** Set the height.
     * \param width The new height value.
     */
    void height(const T &new_height) {
        height_ = new_height;
    }

    /** Get total area of the dimensions.
     * \returns The area (i.e. width * height).
     */
    T getArea() const {
        return width_ * height_;
    }

  private:
    T width_, height_;
};

using Dimension2Di = Dimension2D<int>;
using Dimension2Di16 = Dimension2D<int16_t>;
using Dimension2Di32 = Dimension2D<int32_t>;
using Dimension2Di64 = Dimension2D<int64_t>;
using Dimension2Du = Dimension2D<unsigned int>;
using Dimension2Du16 = Dimension2D<uint16_t>;
using Dimension2Du32 = Dimension2D<uint32_t>;
using Dimension2Du64 = Dimension2D<uint64_t>;
using Dimension2Df = Dimension2D<float>;
using Dimension2Dd = Dimension2D<double>;
}
#endif // RENITY_DIMENSION2D_H_
