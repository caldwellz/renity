/****************************************************
 * Application.h: Game application management class *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_APPLICATION_H_
#define RENITY_APPLICATION_H_

#include "types.h"

namespace renity {
class Window;

/** Encapsulates a game application. */
class RENITY_API Application {
 public:
  Application(int argc, char *argv[]);
  ~Application();

  /* TODO: Someday it may make sense to allow copying/moving Application \
   * objects, but for now, delete the functions to prevent it.
   */
  Application(Application &other) = delete;
  Application(const Application &other) = delete;
  Application &operator=(Application &other) = delete;
  Application &operator=(const Application &other) = delete;

  /** Initialize the application.
   * Loads configuration and resources, creates a window (if not in headless \
   * mode), etc.
   * \returns True if the application was successfully initialized, false \
   * otherwise.
   */
  bool initialize(bool headless = false);

  /** Start the application's main loop.
   * Does not return until ready to exit.
   * \returns 0 on a normal exit, or an error code otherwise.
   */
  int run();

  /** De-initialize the application.
   * Unloads configuration and resources, destroys any windows, etc.
   */
  void destroy();

  /** Get the internal Renity Window of the application.
   * \returns A Window pointer if a window is open and there is a \
   * valid renderer; NULL otherwise.
   */
  Window *getWindow() const;

 private:
  struct Impl;
  Impl *pimpl_;
};
}  // namespace renity
#endif  // RENITY_APPLICATION_H_
