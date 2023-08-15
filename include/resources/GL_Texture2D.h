/****************************************************
 * GL_Texture2D.h: 2D texture resource class        *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Dimension2D.h"
#include "Resource.h"
#include "types.h"

namespace renity {
class Window;
constexpr Uint8 MAX_TEXTURE_UNITS = 16;

/** Encapsulates a drawable texture. */
class RENITY_API GL_Texture2D : public Resource {
 public:
  /** Default constructor. */
  GL_Texture2D();

  /** Default destructor. */
  ~GL_Texture2D();

  // TODO: Implement proper copy/move semantics, if needed.
  GL_Texture2D(GL_Texture2D& other) = delete;
  GL_Texture2D(const GL_Texture2D& other) = delete;
  GL_Texture2D& operator=(GL_Texture2D& other) = delete;
  GL_Texture2D& operator=(const GL_Texture2D& other) = delete;

  /** Load a new image file into the GL_Texture2D.
   * \param src An SDL_RWops stream opened for reading.
   */
  void load(SDL_RWops* src);

  /** Set the texture unit to use when binding/activating this texture.
   * It can either be [0...MAX_TEXTURE_UNITS), or a specific GL_TEXTUREx
   */
  void setTextureUnit(Uint32 unit);

  /** Make this the active texture for the current GL context. */
  void use();

  /** Get the size of the current texture image.
   * \returns A Dimension2D containing the texture's current size in pixels \
   * if the texture is valid; (0, 0) otherwise;
   */
  Dimension2Di getSize() const;

 private:
  struct Impl;
  Impl* pimpl_;
};
using GL_Texture2DPtr = SharedPtr<GL_Texture2D>;
}  // namespace renity
