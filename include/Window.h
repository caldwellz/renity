/****************************************************
 * Window.h: Window management class                *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_WINDOW_H_
#define RENITY_WINDOW_H_

#include <SDL3/SDL_render.h>

#include "Dimension2D.h"
#include "Point2D.h"
#include "types.h"

namespace renity {
/** Encapsulates the idea of a graphical window. */
class RENITY_API Window {
 public:
  Window();
  ~Window();

  /* TODO: Someday it may make sense to allow copying/moving Window \
   * objects, but for now, delete the functions to prevent it.
   */
  Window(Window &other) = delete;
  Window(const Window &other) = delete;
  Window &operator=(Window &other) = delete;
  Window &operator=(const Window &other) = delete;

  /** Get the active (current) Window.
   * \returns A pointer to the last-activated Window, or null if none are open.
   */
  static Window *getActive() {
    extern Window *currentWindow;
    return currentWindow;
  }

  /** Check whether the window is currently open.
   * \returns True if the window is currently open (and hardware-accelerated), \
   * false otherwise.
   */
  bool isOpen() const;

  /** Open/create the window.
   * \returns True if the window was successfully opened (or was already open) \
   * with hardware acceleration, false otherwise.
   */
  bool open();

  /** Close/destroy the window, but keep the settings. */
  void close();

  /** Activate the window.
   * Brings it into focus and makes it the "current" window for any \
   * subsequent rendering.
   * \returns True if the window was activated successfully, false otherwise.
   */
  bool activate();

  /** Update the window.
   * Swaps buffers, processes window events, etc.
   * \returns True if the update succeeded and the window was not closed \
   * (either by code or by the user); false otherwise.
   */
  bool update();

  /** Get the internal SDL_WindowID or the window.
   * \returns The numeric ID of the window, or 0 on failure.
   */
  SDL_WindowID getWindowID() const;

  /** Get the internal SDL_Renderer or the window.
   * \returns An SDL_Renderer pointer if the window is open and there is a \
   * valid renderer; NULL otherwise.
   */
  SDL_Renderer *getRenderer() const;

  /** Get the current title of the window.
   * Can be called at any time (before or after open() or close()).
   * \returns The current (or previously-used, if now closed) window title.
   */
  String title() const;

  /** Set the title of the window.
   * Can be called at any time (before or after open() or close()).
   * \param title The new window title.
   */
  void title(const String new_title);

  /** Get the window's current position.
   * \returns A Point2D containing the window's current position, in screen \
   * coordinates (i.e. pixels, except on HighDPI devices).
   */
  Point2Di32 position() const;

  /** Set the window's position.
   * Can be called at any time (before or after open() or close()).
   * Uses at least an i32 so one can pass SDL_WINDOWPOS_* flags.
   * \param new_pos Window position relative to the top-left of the display, \
   * in screen coordinates (i.e. pixels, except on HighDPI devices).
   */
  void position(const Point2Di32 &new_pos);

  /** Set the window's position to be centered on the display.
   * Can be called any time (before or after open() or close()).
   */
  void centerPosition();

  /** Get the window's size.
   * Uses an i32 for consistency, even though a u16 should be quite sufficient.
   * Can be called any time (before or after open() or close()), but may not be
   * accurate before the window is opened if planning to use the native desktop
   * resolution.
   * \returns A Dimension2D containing the window's current size, in screen
   * coordinates (i.e. pixels, except on HighDPI devices).
   */
  Dimension2Di32 size() const;

  /** Get the window's size, in actual pixels.
   * Can be called any time (before or after open() or close()), but may not be
   * accurate before the window is opened on HighDPI devices and/or if planning
   * to use the native desktop resolution. \returns A Dimension2D containing
   * the window's current size, in pixels.
   */
  Dimension2Di32 sizeInPixels() const;

  /** Set the window's size.
   * Will change the video mode if the window is currently fullscreen.
   * Can be called any time (before or after open() or close()).
   * \param new_size A window size, in screen coordinates (i.e. pixels, except
   * on HighDPI devices). A value of 0 for the width or height will be ignored.
   * \returns True on success, false on failure.
   */
  bool size(const Dimension2Di32 &new_size);

  /** Get the window's fullscreen state.
   * Can be called any time (before or after open() or close()).
   * \returns True if the window is configured to be fullscreen, false
   * otherwise.
   */
  bool isFullscreen() const;

  /** Attempt to enable or disable fullscreen mode.
   * Can be called any time (before or after open() or close()).
   * \param fullscreen True to make the window fullscreen, false to make it \
   * windowed.
   * \param use_native_resolution True to use the screen's native resolution, \
   * false to use the configured window width and height. The preconfigured \
   * resolution may not be respected if it is not a supported video mode.
   * \returns True on success, false on failure.
   */
  bool useFullscreen(bool fullscreen, bool use_native_resolution);

 private:
  struct Impl;
  Impl *pimpl_;
  friend int windowEventProcessor(void *userdata, SDL_Event *event);
};
}  // namespace renity
#endif  // RENITY_WINDOW_H_
