/***************************************************
 * Sprite.h: Stateful texture-based sprite class    *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Sprite.h"

#include <errno.h>
#include <fenv.h>
#include <math.h>

#include "ResourceManager.h"
#include "config.h"
#include "types.h"

const double radiansToDegrees = 180.0 / 3.141593;

namespace renity {
struct Sprite::Impl {
  /** Defaults to full-screen VSynced mode at the native screen resolution. */
  Impl() {
    tex = nullptr;
    scale = Dimension2Dd(1.0, 1.0);
    direction = Point2Dd(0.0, 1.0);  // 0 degrees
    x = 0.0;
    y = 0.0;
    rotation = 0.0;
    speed = 1.0;
    usingDefaultOrigin = true;
    flipHorizontal = false;
    flipVertical = false;
  }

  void updateImageOrigin() {
    if (usingDefaultOrigin) {
      // Use the default SDL origin (dstrect.w / 2, dstrect.h / 2)
      origin.x(destRect.width() / 2);
      origin.y(destRect.height() / 2);
    }
  }

  TexturePtr tex;
  Point2Di origin;
  Dimension2Dd scale;
  Rect2Di srcRect, destRect;
  Point2Dd direction;
  double x, y, rotation, speed;
  bool usingDefaultOrigin, flipHorizontal, flipVertical;
};

RENITY_API Sprite::Sprite() {
  // Initialize with default settings
  pimpl_ = new Impl();
}

RENITY_API Sprite::Sprite(TexturePtr& texture) {
  pimpl_ = new Impl();
  setTexture(texture);
}

RENITY_API Sprite::Sprite(const char* path) {
  pimpl_ = new Impl();
  setTexture(path);
}

RENITY_API Sprite::~Sprite() { delete pimpl_; }

RENITY_API void Sprite::setTexture(TexturePtr& texture) {
  pimpl_->tex = texture;

  // Reset the image scale and clipping
  if (pimpl_->tex) {
    useDefaultOrigin();
    setImageScale(pimpl_->scale);
    pimpl_->srcRect.x(0);
    pimpl_->srcRect.y(0);
    pimpl_->srcRect.size(pimpl_->tex->getSize());
  }
}

RENITY_API void Sprite::setTexture(const char* path) {
  if (!path) {
    pimpl_->tex.reset();
    return;
  }
  TexturePtr texture = ResourceManager::getActive()->get<Texture>(path);
  setTexture(texture);
}

RENITY_API Dimension2Dd Sprite::getImageScale() const { return pimpl_->scale; }

RENITY_API void Sprite::setImageScale(const Dimension2Dd& scale) {
  pimpl_->scale = scale;
  if (pimpl_->tex) {
    Dimension2Di imageSize = pimpl_->tex->getSize();
    pimpl_->destRect.width(scale.width() * imageSize.width());
    pimpl_->destRect.height(scale.height() * imageSize.height());
    pimpl_->updateImageOrigin();
  }
}

RENITY_API Rect2Di Sprite::getImageClip() const { return pimpl_->srcRect; }

RENITY_API void Sprite::setImageClip(const Rect2Di& sourceClip) {
  pimpl_->srcRect = sourceClip;
}

RENITY_API Point2Di Sprite::getImageOrigin() const { return pimpl_->origin; }

RENITY_API void Sprite::setImageOrigin(const Point2Di& origin) {
  pimpl_->usingDefaultOrigin = false;
  pimpl_->origin = origin;
}

RENITY_API void Sprite::useDefaultOrigin() {
  pimpl_->usingDefaultOrigin = true;
  pimpl_->updateImageOrigin();
}

RENITY_API double Sprite::getImageRotation() const { return pimpl_->rotation; }

RENITY_API void Sprite::setImageRotation(const double& angle) {
  pimpl_->rotation = angle;
}

RENITY_API void Sprite::flipImageHorizontal() {
  pimpl_->flipHorizontal = !pimpl_->flipHorizontal;
}

RENITY_API void Sprite::flipImageVertical() {
  pimpl_->flipVertical = !pimpl_->flipVertical;
}

RENITY_API void Sprite::unflipImage() {
  pimpl_->flipHorizontal = false;
  pimpl_->flipVertical = false;
}

RENITY_API Point2Di Sprite::getPosition() const {
  return pimpl_->destRect.position();
}

RENITY_API void Sprite::setPosition(const Point2Di& position) {
  pimpl_->destRect.position(position);
  pimpl_->x = position.x();
  pimpl_->y = position.y();
}

RENITY_API double Sprite::getMoveHeading() const {
  // atan2 normally takes (y, x), but we reverse them for the sake of going
  // clockwise
  double angle =
      atan2(pimpl_->direction.x(), pimpl_->direction.y()) * radiansToDegrees;
  if (angle < 0.0) angle += 360.0;

  // Clear any domain errors that may have happened (i.e. passing (0, 0) to
  // atan2). The expectation is that it will simply have returned 0 in that
  // case.
  if (math_errhandling & MATH_ERRNO) errno = 0;
  if (math_errhandling & MATH_ERREXCEPT) feclearexcept(FE_ALL_EXCEPT);

  return angle;
}

RENITY_API void Sprite::setMoveHeading(const double& angle) {
  // x = sine & y = cosine so that we end up going clockwise from the top.
  pimpl_->direction.x(sin(angle / radiansToDegrees));
  pimpl_->direction.y(cos(angle / radiansToDegrees));
}

RENITY_API Point2Dd Sprite::getMoveDirection() const {
  return pimpl_->direction;
}

RENITY_API void Sprite::setMoveDirection(const Point2Dd& direction) {
  pimpl_->direction = direction;
}

RENITY_API double Sprite::getMoveSpeed() const { return pimpl_->speed; }

RENITY_API void Sprite::setMoveSpeed(const double& speed) {
  pimpl_->speed = speed;
}

RENITY_API void Sprite::bounceHorizontal(bool flipImage) {
  if (flipImage) flipImageHorizontal();

  // Reflect across the Y axis by negating the angle, effectively (360-angle)
  double angle = getMoveHeading();
  setMoveHeading(-angle);
}

RENITY_API void Sprite::bounceVertical(bool flipImage) {
  if (flipImage) flipImageVertical();

  // Reflect across the X axis
  double angle = getMoveHeading();
  setMoveHeading(180 - angle);
}

RENITY_API void Sprite::move() {
  pimpl_->x += pimpl_->direction.x() * pimpl_->speed;
  pimpl_->y += pimpl_->direction.y() * pimpl_->speed;
  pimpl_->destRect.position(Point2Di((int)pimpl_->x, (int)pimpl_->y));
}

RENITY_API bool Sprite::draw() {
  if (pimpl_->tex) {
    Rect2Di originRect = pimpl_->destRect;
    originRect.x(originRect.x() - pimpl_->origin.x());
    originRect.y(originRect.y() - pimpl_->origin.y());
    return pimpl_->tex->draw(&pimpl_->srcRect, &originRect, pimpl_->rotation,
                             &pimpl_->origin, pimpl_->flipHorizontal,
                             pimpl_->flipVertical);
  }

  return false;
}
}  // namespace renity
