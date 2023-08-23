/****************************************************
 * Rect2D.h: Rectangle template                     *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_RECT2D_H_
#define RENITY_RECT2D_H_

#include <SDL3/SDL_rect.h>

#include "Dimension2D.h"
#include "Point2D.h"

namespace renity {

/** Expresses a rectangle containing both a position and set of dimensions. */
template <typename T>
class Rect2D : public Dimension2D<T>, public Point2D<T> {
 public:
  /** Default constructor */
  Rect2D() = default;

  /** Parameterized constructor that inisitalizes the Rect with the given
   * values. \param x The initial x coordinate. \param y The initial y
   * coordinate. \param width The initial width. \param height The initial
   * height.
   */
  Rect2D(const T x, const T y, const T width, const T height)
      : Dimension2D<T>(width, height), Point2D<T>(x, y) {}

  /** Get the position of the rectangle.
   * \returns A Point2D containing the current X/Y position.
   */
  Point2D<T> position() const { return Point2D<T>(this->x_, this->y_); }

  /** Set the position of the rectangle.
   * \param newPosition A Point2D containing the new X/Y position.
   */
  void position(const Point2D<T> &newPosition) {
    this->x = newPosition.x_;
    this->y = newPosition.y_;
  }

  /** Get the size (dimensions) of the rectangle.
   * \returns A Dimension2D containing the current width and height.
   */
  Dimension2D<T> size() const {
    return Dimension2D<T>(this->width_, this->height_);
  }

  /** Set the size (dimensions) of the rectangle.
   * \param newSize A Dimension2D containing the new width and height.
   */
  void size(const Dimension2D<T> &newSize) {
    this->width_ = newSize.width_;
    this->height = newSize.height_;
  }

  /** Check if any point within the rectangle is within the bounds of another
   * given rectangle. */
  bool intersects(const Rect2D<T> &rhs) {
    if (rhs.x_ > this->x_ + this->width_ || rhs.y_ > this->y_ + this->height_ ||
        this->x_ > rhs.x_ + rhs.width_ || this->y_ > rhs.y_ + rhs.height_)
      return false;
    return true;
  }

  /** Create a Rect2D of the given size around a center point. */
  static Rect2D<T> getFromCentroid(const Point2D<T> center,
                                   const Dimension2D<T> size) {
    T radiusX = size.width() / (T)2;
    T radiusY = size.height() / (T)2;
    return Rect2D<T>(center.x() - radiusX, center.y() - radiusY, size.width(),
                     size.height());
  }

  /** Get the center point of the rectangle. */
  Point2D<T> getCentroid() {
    return Point2D<T>(this->x_ + this->width_ / (T)2,
                      this->y_ + this->height_ / (T)2);
  }

  /** Scale the position and dimensions around the center.
   * \returns A reference to this Rect2D, for operation chaining.
   */
  Rect2D<T> &scaleFromCenter(float scale) {
    Point2D<T> center = getCentroid();
    float adjWidth = this->width_ * scale, adjHeight = this->height_ * scale;
    Dimension2D<T> adjustedSize((T)adjWidth, (T)adjHeight);
    *this = getFromCentroid(center, adjustedSize);
    return *this;
  }

  /** Utility function that returns the current rectangle as an SDL_Rect.
   * \todo Determine/perform more appropriate conversions.
   * \returns An SDL_Rect representing the current state of the rectangle.
   */
  SDL_Rect toSDLRect() const {
    SDL_Rect rect;
    rect.x = (int)this->x_;
    rect.y = (int)this->y_;
    rect.w = (int)this->width_;
    rect.h = (int)this->height_;
    return rect;
  }

  /** Utility function that returns the current rectangle as an SDL_FRect.
   * \todo Determine/perform more appropriate conversions.
   * \returns An SDL_Rect representing the current state of the rectangle.
   */
  SDL_FRect toSDLFRect() const {
    SDL_FRect rect;
    rect.x = (float)this->x_;
    rect.y = (float)this->y_;
    rect.w = (float)this->width_;
    rect.h = (float)this->height_;
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
}  // namespace renity
#endif  // RENITY_RECT2D_H_
