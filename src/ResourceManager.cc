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

#ifdef RENITY_DEBUG
#define DMON_IMPL
#include "3rdparty/dmon/dmon.h"
#endif

namespace renity {
using StrongRes = SharedPtr<Resource>;
using WeakRes = WeakPtr<Resource>;
ResourceManager *currentManager = nullptr;
struct ResourceManager::Impl {
  HashTable<const char *, WeakRes> map;
#ifdef RENITY_DEBUG
  // Implement hot-reload in debug mode
  dmon_watch_id watchId;
  Impl() { watchId.id = 0; }
  static void dmonCallback(dmon_watch_id watch_id, dmon_action action,
                           const char *rootdir, const char *filepath,
                           const char *oldFilepath, void *user) {
    (void)(watch_id);
    (void)(rootdir);
    int isDir = PHYSFS_isDirectory(filepath);
    if (!isDir) {
      Impl *pimpl_ = (Impl *)(user);
      WeakRes &weak = pimpl_->map.get(filepath);
      const char *statusStr = weak.expired() ? "unused" : "active";
      switch (action) {
        case DMON_ACTION_CREATE:
          SDL_Log("CREATE: [baseDir]/%s (%s)\n", filepath, statusStr);
          if (!weak.expired())
            weak.lock()->load(PHYSFSRWOPS_openRead(filepath));
          break;
        case DMON_ACTION_DELETE:
          SDL_Log("DELETE: [baseDir]/%s (%s)\n", filepath, statusStr);
          if (!weak.expired()) weak.lock()->load(nullptr);
          break;
        case DMON_ACTION_MODIFY:
          SDL_Log("MODIFY: [baseDir]/%s (%s)\n", filepath, statusStr);
          if (!weak.expired())
            weak.lock()->load(PHYSFSRWOPS_openRead(filepath));
          break;
        case DMON_ACTION_MOVE:
          WeakRes &weakOld = pimpl_->map.get(oldFilepath);
          SDL_Log("MOVE: [baseDir]/%s (%s) -> [baseDir]/%s (%s)\n", oldFilepath,
                  weakOld.expired() ? "unused" : "active", filepath, statusStr);
          if (!weakOld.expired()) weakOld.lock()->load(nullptr);
          if (!weak.expired())
            weak.lock()->load(PHYSFSRWOPS_openRead(filepath));
          break;
      }
    }
  }
#endif
};

RENITY_API ResourceManager::ResourceManager() {
  pimpl_ = new Impl();
  activate();
}

RENITY_API ResourceManager::~ResourceManager() {
#ifdef RENITY_DEBUG
  if (pimpl_->watchId.id > 0) {
    dmon_unwatch(pimpl_->watchId);
  }
#endif
  clear();
  delete this->pimpl_;
}

void ResourceManager::activate() {
#ifdef RENITY_DEBUG
  if (pimpl_->watchId.id == 0) {
    const char *baseDir = PHYSFS_getBaseDir();
    if (baseDir) {
      pimpl_->watchId = dmon_watch(baseDir, pimpl_->dmonCallback,
                                   DMON_WATCHFLAGS_RECURSIVE, pimpl_);
      SDL_Log("DMON: Id %u is watching dir '%s'\n", pimpl_->watchId.id,
              baseDir);
    } else {
      SDL_Log("ResourceManager::activate() triggered with invalid baseDir.\n");
    }
  }
#endif
  currentManager = this;
}

void ResourceManager::clear() {
  if (currentManager == this) {
    currentManager = nullptr;
  }
  pimpl_->map.clear();
}

RENITY_API SharedPtr<Resource> ResourceManager::getOrCreate(
    const char *path, Resource *(*factory)(SDL_RWops *)) {
  WeakRes &weak = pimpl_->map.get(path);
  if (!weak.expired()) {
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_APPLICATION,
        "ResourceManager::getOrCreate: Returning EXISTING ptr for '%s'\n",
        path);
    return weak.lock();
  }

  SDL_RWops *ops = PHYSFSRWOPS_openRead(path);
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                 "ResourceManager::getOrCreate: CREATING ptr for '%s' (%s)\n",
                 path, ops ? "valid" : "NOT valid");
  const StrongRes &strong = StrongRes(factory(ops));
  if (!ops) {
    SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM,
                "ResourceManager::getOrCreate: PHYSFSRWOPS_openRead(\"%s\") "
                "failed: '%s'\n",
                path, SDL_GetError());
  }
  weak = strong;
  return strong;
}
}  // namespace renity