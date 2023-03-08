/****************************************************
 * ResourceManager.cc: Resource loading and caching *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "ResourceManager.h"

#include <SDL3/SDL.h>

#include "HashTable.h"
#include "utils/physfsrwops.h"

namespace renity {
using StrongRes = SharedPtr<Resource>;
using WeakRes = WeakPtr<Resource>;
ResourceManager *currentManager = nullptr;
struct ResourceManager::Impl {
  DualHashTable<const char *, size_t, WeakRes> map;
};

RENITY_API ResourceManager::ResourceManager() {
  pimpl_ = new Impl();
  activate();
}

RENITY_API ResourceManager::~ResourceManager() {
  clear();
  delete this->pimpl_;
}

void ResourceManager::activate() { currentManager = this; }

void ResourceManager::clear() {
  if (currentManager == this) {
    currentManager = nullptr;
  }
  pimpl_->map.clear();
}

RENITY_API SharedPtr<Resource> ResourceManager::getOrCreate(
    const char *path, size_t type, Resource *(*factory)(SDL_RWops *)) {
  WeakRes &weak = pimpl_->map.get(path, type);
  if (!weak.expired()) {
    SDL_LogDebug(
        SDL_LOG_CATEGORY_SYSTEM,
        "ResourceManager::getOrCreate: Returning EXISTING ptr for '%s':%llu\n",
        path, type);
    return weak.lock();
  }

  SDL_RWops *ops = PHYSFSRWOPS_openRead(path);
  SDL_LogDebug(SDL_LOG_CATEGORY_SYSTEM,
               "ResourceManager::getOrCreate: CREATING ptr for '%s':%llu\n",
               ops ? path : "(invalid:default)", type);
  const StrongRes &strong = StrongRes(factory(ops));
  if (!ops) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                 "ResourceManager::getOrCreate: PHYSFSRWOPS_openRead(\"%s\") "
                 "failed: '%s'\n",
                 path, SDL_GetError());
  }
  weak = strong;
  return strong;
}
}  // namespace renity
