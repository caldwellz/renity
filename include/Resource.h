/****************************************************
 * Resource.h: Resource virtual base class          *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include <SDL3/SDL_rwops.h>

#include "types.h"

namespace renity {
class ResourceManager;

typedef void (*ResourceLoadCallback)(void *userdata);

class RENITY_API Resource {
 public:
  Resource() : cb_(nullptr), userdata_(nullptr){};
  virtual ~Resource(){};

  /* TODO: Someday it may make sense to allow copying/moving Resource
   * objects, but for now, delete the functions to prevent it.
   */
  Resource(Resource &other) = delete;
  Resource(const Resource &other) = delete;
  Resource &operator=(Resource &other) = delete;
  Resource &operator=(const Resource &other) = delete;

  /** Set a callback to use after ResourceManager reloads a loaded Resource. */
  inline void setReloadCallback(const ResourceLoadCallback &cb,
                                void *userdata = nullptr) {
    cb_ = cb;
    userdata_ = userdata;
  }

 protected:
  friend class ResourceManager;
  ResourceLoadCallback cb_;
  void *userdata_;

  /** Load the resource from an SDL_RWops stream.
   * Derived classes should start in a usable, empty state on construction.
   * This function then may (or may not) be called at any time to signal e.g. a
   * replaced or deleted file. ResourceManager will automatically run cb_ after.
   * @param src A read-only SDL_RWops stream. If NULL, a derived class should
   * clean up as needed and switch to a default state (such as a blank texture).
   */
  virtual void load(SDL_RWops *src) = 0;
};
using ResourcePtr = SharedPtr<Resource>;
}  // namespace renity
