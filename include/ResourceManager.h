/****************************************************
 * ResourceManager.h: Resource loading and caching  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include <SDL3/SDL_rwops.h>

#include "Resource.h"
#include "types.h"

namespace renity {
class RENITY_API ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  /* TODO: Someday it may make sense to allow copying/moving ResourceManager
   * objects, but for now, delete the functions to prevent it.
   */
  ResourceManager(ResourceManager &other) = delete;
  ResourceManager(const ResourceManager &other) = delete;
  ResourceManager &operator=(ResourceManager &other) = delete;
  ResourceManager &operator=(const ResourceManager &other) = delete;

  /** Get a Resource from a PhysFS file, or generate a blank one in memory.
   * Keeps it in a context-specific cache.
   * @param path Filename to read, in platform-independent notation,
   * or a name in <angle brackets> to generate a cached in-memory Resource.
   * @return A Resource representing a file's data, or default data if there was
   * an error or a request to generate a temporary Resource.
   */
  template <typename T>
  SharedPtr<T> get(const char *path) {
    requireBaseOf<Resource, T>();
    SharedPtr<Resource> res = getOrCreate(path, &createResource<T>);
    return dynamicPointerCast<T>(res);
  }

  /** Get the active (current) ResourceManager.
   * \returns A pointer to the last-activated ResourceManager, or null if none
   * are valid.
   */
  static ResourceManager *getActive();

  /** Activate this ResourceManager.
   * Makes it the "current" manager for any subsequent resource operations.
   * This is useful for e.g. GL_Shaders, which are Window-specific.
   */
  void activate();

  /** Update the ResourceManager.
   * Reloads any cached resources which have changed on disk.
   * Rendering threads should use this to reload Window-specific resources.
   */
  void update();

  /** Clear this ResourceManager's contents.
   * Effectively de-activates it and removes everything from the resource cache.
   */
  void clear();

 protected:
  template <typename T>
  static Resource *createResource(SDL_RWops *ops) {
    // Should return a default resource if the file was not opened successfully.
    T *res = new T();
    res->load(ops);
    return res;
  }
  SharedPtr<Resource> getOrCreate(const char *path,
                                  Resource *(*factory)(SDL_RWops *));

 private:
  struct Impl;
  Impl *pimpl_;
};
}  // namespace renity
