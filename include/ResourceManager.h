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

  /** Get a Resource from a PhysFS file.
   * Keeps it in a context-specific cache.
   * @param path Filename to read, in platform-independent notation.
   * @return A Resource representing the file's data, or default data on an
   * error.
   */
  template <typename T>
  SharedPtr<T> get(const char *path) {
    requireBaseOf<Resource, T>();
    SharedPtr<Resource> res =
        getOrCreate(path, typeHash(T), &createResource<T>);
    return staticPointerCast<T>(res);
  }

  /** Get the active (current) ResourceManager.
   * \returns A pointer to the last-activated ResourceManager, or null if none
   * are valid.
   */
  static ResourceManager *getActive() {
    extern ResourceManager *currentManager;
    return currentManager;
  }

  /** Activate this ResourceManager.
   * Makes it the "current" manager for any subsequent resource operations.
   * This is useful for e.g. Textures, which are Window-specific.
   */
  void activate();

  /** Clear this ResourceManager's contents.
   * Effectively de-activates it and removes everything from the resource cache.
   */
  void clear();

 protected:
  template <typename T>
  static Resource *createResource(SDL_RWops *ops) {
    // Return a default resource unless the data file was opened successfully.
    T *res = new T();
    if (ops) {
      res->load(ops);
    }
    return res;
  }
  SharedPtr<Resource> getOrCreate(const char *path, size_t type,
                                  Resource *(*factory)(SDL_RWops *));

 private:
  struct Impl;
  Impl *pimpl_;
};
}  // namespace renity
