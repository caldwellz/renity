/****************************************************
 * Sprite.h: Stateful texture-based sprite class    *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_SPRITE_H_
#define RENITY_SPRITE_H_

#include "Dimension2D.h"
#include "Point2D.h"
#include "Rect2D.h"
#include "types.h"

namespace renity {
class Texture;
class Window;
/** Encapsulates a movable/drawable Sprite. */
class RENITY_API Sprite {
 public:
  /** Default constructor.
   * Does not use any texture image.
   */
  Sprite();

  /** Basic constructor.
   * \param texture The Texture that should be used for this Sprite.
   */
  Sprite(Texture& texture);

  /** Texture loader constructor.
   * Attempts to load and use a texture image using the given path.
   * \param window The Window whose renderer should be used.
   * \param path A PhysFS path to an image file.
   */
  Sprite(const Window& window, const String& path);

  /** Default destructor. */
  ~Sprite();

  // TODO: Implement proper copy/move semantics, if needed.
  Sprite(Sprite& other) = delete;
  Sprite(const Sprite& other) = delete;
  Sprite& operator=(Sprite& other) = delete;
  Sprite& operator=(const Sprite& other) = delete;

  /** Set the Texture to use for drawing.
   * Updates the image origin only if the default one is currently is use.
   * Resets any custom clipping currently in use.
   * \param texture The new Texture image to use. Can be NULL for no image.
   */
  void setTexture(Texture* texture);

  /** Get the current scale of the texture image.
   * \returns A scale multiplier (1.0 for width & height means actual image
   * size).
   */
  Dimension2Dd getImageScale() const;

  /** Set the current scale of the texture image.
   * Updates the image origin only if the default one is currently is use.
   * \param scale A scale multiplier. \
   *  A scale of 1.0 for a dimension means to use the actual image size.
   */
  void setImageScale(const Dimension2Dd& scale);

  /** Get the image's clipping rectangle.
   * \returns A rectangle indicating the subsection of the image to draw.
   */
  Rect2Di getImageClip() const;

  /** Set the image's clipping rectangle.
   * \param sourceClip A rectangle indicating the subsection of the image to \
   * draw.
   */
  void setImageClip(const Rect2Di& sourceClip);

  /** Get the origin point of the image.
   * \returns The origin point around which the Sprite image will rotate.
   */
  Point2Di getImageOrigin() const;

  /** Set the origin point of the image.
   * \param origin The origin point around which the Sprite image will rotate.
   */
  void setImageOrigin(const Point2Di& origin);

  /** Use the default origin (the center) for rotating the image. */
  void useDefaultOrigin();

  /** Get the image's rotation angle.
   * \returns The image's rotation angle, in degrees (0.0 - 360.0).
   */
  double getImageRotation() const;

  /** Set the image's rotation angle.
   * \param angle The image's rotation angle, in degrees (0.0 - 360.0).
   */
  void setImageRotation(const double& angle);

  /** Flip the image horizontally.
   * If the image is already flipped, this will flip it back.
   */
  void flipImageHorizontal();

  /** Flip the image vertically.
   * If the image is already flipped, this will flip it back.
   */
  void flipImageVertical();

  /** Remove any flipping from the image. */
  void unflipImage();

  /** Get the sprite's current position.
   * \returns A Point2D containing the sprite's current position, in screen \
   * space pixels.
   */
  Point2Di getPosition() const;

  /** Set the sprite's current position.
   * \param position A Point2D containing the sprite's new position, in \
   * screen space pixels.
   */
  void setPosition(const Point2Di& position);

  /** Get the current movement direction as an angle.
   * \returns The current movement angle in degrees clockwise from the top, \
   * with a range of [0.0 - 360.0].
   */
  double getMoveHeading() const;

  /** Set the movement direction using an angle.
   * \param angle The new movement angle in degrees clockwise from the top, \
   * with a range of [0.0 - 360.0]. Other values are acceptable, but will be \
   * translated into this range.
   */
  void setMoveHeading(const double& angle);

  /** Get the current movement direction as an X/Y slope.
   * \returns The direction/heading as X/Y values in a range of [-1.0, 1.0].
   */
  Point2Dd getMoveDirection() const;

  /** Set the movement direction using X/Y values.
   * \param direction The new direction/heading as X/Y values in a range of \
   * [-1.0, 1.0]. Values outside this range are acceptable, but will not \
   * update the movement speed accordingly.
   */
  void setMoveDirection(const Point2Dd& direction);

  /** Get the current movement speed.
   * \returns The current movement speed, in pixels per move().
   */
  double getMoveSpeed() const;

  /** Set the current movement speed.
   * \param speed The new movement speed, in pixels per move().
   */
  void setMoveSpeed(const double& speed);

  /** "Bounce" the sprite's movement horizontally.
   * Reflect the current movement direction across the Y axis, as if the \
   * sprite bounced into something horizontally.
   * \param flipImage Whether to also flip the image accordingly.
   */
  void bounceHorizontal(bool flipImage = false);

  /** "Bounce" the sprite's movement vertically.
   * Reflect the current movement direction across the X axis, as if the \
   * sprite bounced into something vertically.
   * \param flipImage Whether to also flip the image accordingly.
   */
  void bounceVertical(bool flipImage = false);

  /** Move the sprite.
   * Updates the sprite's position, moving along the current direction/heading \
   * using the current speed.
   */
  void move();

  /** Draw the Sprite using its configured Texture.
   * \returns True if the image was drawn successfully, false otherwise.
   */
  bool draw();

 private:
  struct Impl;
  Impl* pimpl_;
};
}  // namespace renity
#endif  // RENITY_SPRITE_H_
