/****************************************************
 * Texture.h: Texture management class              *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_TEXTURE_H_
#define RENITY_TEXTURE_H_

#include "Point2D.h"
#include "Rect2D.h"
#include "types.h"

namespace renity {
class Window;

/** Encapsulates a drawable texture. */
class RENITY_API Texture {
 public:
  /** Default constructor. */
  Texture();

  /** Basic constructor.
   * Does not load a texture image (and won't be valid until load() is called).
   * \param window The Window whose renderer should be used for this Texture.
   */
  Texture(const Window& window);

  /** Loader constructor.
   * Attempts to load a texture image using the given path. Check isValid()
   * after. \param window The Window whose renderer should be used for this
   * Texture. \param path A PhysFS path to an image file.
   */
  Texture(const Window& window, const String& path);

  /** Default destructor. */
  ~Texture();

  // TODO: Implement proper copy/move semantics, if needed.
  Texture(Texture& other) = delete;
  Texture(const Texture& other) = delete;
  Texture& operator=(Texture& other) = delete;
  Texture& operator=(const Texture& other) = delete;

  /** Load a new image file into the Texture.
   * \param path A PhysFS path to an image file.
   * \returns True if the new image was successfully loaded, false otherwise.
   */
  bool load(const String path);

  /** Unload (destroy/invalidate) the texture image. */
  void unload();

  /** Enable a transparency color key
   * \param keyPosition The 2D coordinates of a transparent pixel.
   * \returns True if transparency was enabled successfully, false otherwise.
   */
  bool enableColorKey(const Point2Di& keyPosition);

  /** Disable a transparency color key */
  void disableColorKey();

  /** Check whether a transparency color key is enabled.
   * \returns True if a transparency color key pixel is enabled, false
   * otherwise.
   */
  bool isColorKeyEnabled();

  /** Check whether the texture is valid.
   * \returns True if the texture is valid (meaning an image has been loaded) \
   * and can be used for drawing; false otherwise.
   */
  bool isValid() const;

  /** Get the texture's underlying image path.
   * \returns A String containing the PhysFS path of the current texture image.
   */
  String getImagePath() const;

  /** Get the size of the current texture image.
   * \returns A Dimension2D containing the texture's current size in pixels \
   * if the texture is valid; (0, 0) otherwise;
   */
  Dimension2Di getSize() const;

  /** Set the Window (and thus renderer) used by this Texture.
   * Will unload and reload the Texture image if one was already loaded.
   * \param window The Window whose renderer should be used for this Texture.
   * \returns True if the Window was valid and the Texture was reloaded \
   * successfully (or was not loaded previously); false otherwise.
   */
  bool setWindow(const Window& window);

  /** Draw the Texture image on to its configured Window.
   * \param source The section of the image to copy, or NULL for the entirety.
   * \param dest The area of the Window to copy the selected section of the \
   * image to, or NULL to use the entire Window dimensions. The image will be \
   * stretched as needed to fit the destination.
   * \param angle The amount of rotation to apply to dest, in clockwise degrees.
   * \param origin The point around which to rotate dest, or NULL for the
   * center. \param flipHorizontal Whether to flip the image horizontally.
   * \param flipVertical Whether to flip the image vertically.
   * \returns True if the image was drawn successfully, false otherwise.
   */
  bool draw(const Rect2Di* source, const Rect2Di* dest, const double& angle = 0,
            const Point2Di* origin = nullptr,
            const bool& flipHorizontal = false,
            const bool& flipVertical = false);

 private:
  struct Impl;
  Impl* pimpl_;
};
}  // namespace renity
#endif  // RENITY_TEXTURE_H_
