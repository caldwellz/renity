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
ResourceManager *currentResourceManager = nullptr;
struct ResourceUpdate {
  String path;
  bool fileStillValid;
};

struct ResourceManager::Impl {
  HashTable<String, WeakRes> map;
#ifdef RENITY_DEBUG
  // Implement hot-reload in debug mode
  dmon_watch_id watchId;
  Vector<ResourceUpdate> updates;
  SDL_Mutex *updateLock;
  Impl() {
    watchId.id = 0;
    updateLock = SDL_CreateMutex();
  }
  ~Impl() { SDL_DestroyMutex(updateLock); }
  static void dmonCallback(dmon_watch_id watch_id, dmon_action action,
                           const char *rootdir, const char *filepath,
                           const char *oldFilepath, void *user) {
    (void)(watch_id);
    (void)(rootdir);
    String absPathStr("/");
    absPathStr += filepath;
    const char *absPath = absPathStr.c_str();
    if (PHYSFS_isDirectory(absPath)) {
      SDL_Log("DIRECTORY action: [baseDir]%s", absPath);
      return;
    }

    String absOldPathStr("/");
    Impl *pimpl_ = (Impl *)(user);
    SDL_LockMutex(pimpl_->updateLock);
    bool known = pimpl_->map.exists(absPathStr);
    WeakRes &weak = pimpl_->map.get(absPathStr);
    const char *statusStr =
        weak.expired() ? (known ? "known" : "unused") : "active";
    ResourcePtr res = weak.lock();
    bool fileStillValid = true;
    switch (action) {
      case DMON_ACTION_CREATE:
        SDL_Log("CREATE: [baseDir]%s (%s)", absPath, statusStr);
        break;
      case DMON_ACTION_DELETE:
        SDL_Log("DELETE: [baseDir]%s (%s)", absPath, statusStr);
        fileStillValid = false;
        break;
      case DMON_ACTION_MODIFY:
        SDL_Log("MODIFY: [baseDir]%s (%s)", absPath, statusStr);
        break;
      case DMON_ACTION_MOVE:
        absOldPathStr += oldFilepath;
        WeakRes &weakOld = pimpl_->map.get(absOldPathStr);
        SDL_Log("MOVE: [baseDir]%s (%s) -> [baseDir]%s (%s)",
                absOldPathStr.c_str(), weakOld.expired() ? "unused" : "active",
                absPath, statusStr);
        if (!weakOld.expired()) {
          pimpl_->updates.push_back({absOldPathStr, false});
        }
        break;
    }
    if (!weak.expired()) {
      pimpl_->updates.push_back({absPathStr, fileStillValid});
    }
    SDL_UnlockMutex(pimpl_->updateLock);
  }
#endif
};

RENITY_API ResourceManager::ResourceManager() {
  pimpl_ = new Impl();
  if (!currentResourceManager) currentResourceManager = this;
}

RENITY_API ResourceManager::~ResourceManager() {
#ifdef RENITY_DEBUG
  if (pimpl_->watchId.id > 0) {
    dmon_unwatch(pimpl_->watchId);
  }
#endif
  clear();
  if (currentResourceManager == this) currentResourceManager = nullptr;
  delete this->pimpl_;
}

RENITY_API ResourceManager *ResourceManager::getActive() {
  return currentResourceManager;
}

RENITY_API void ResourceManager::activate() {
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
  currentResourceManager = this;
}

RENITY_API void ResourceManager::update() {
#ifdef RENITY_DEBUG
  // Don't wait for the file watcher to finish, just try again next frame
  if (SDL_TryLockMutex(pimpl_->updateLock) != 0) {
    return;
  }

  for (auto update : pimpl_->updates) {
    ResourcePtr res = pimpl_->map.get(update.path).lock();
    SDL_RWops *ops = nullptr;
    if (update.fileStillValid) {
      ops = PHYSFSRWOPS_openRead(update.path.c_str());
    }
    res->load(ops);
    if (res->cb_) res->cb_(res->userdata_);
  }
  pimpl_->updates.clear();
  SDL_UnlockMutex(pimpl_->updateLock);
#endif
}

RENITY_API void ResourceManager::clear() {
  if (currentResourceManager == this) {
    currentResourceManager = nullptr;
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

  SDL_RWops *ops = nullptr;
  if (!path) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "ResourceManager::getOrCreate: No path specified - creating "
                "empty Resource.\n");
  } else if (path[0] != '<') {  // internal, non-file resources use <names>
    ops = PHYSFSRWOPS_openRead(path);
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                   "ResourceManager::getOrCreate: CREATING ptr for '%s' (%s)\n",
                   path, ops ? "valid" : "NOT valid");
    if (!ops) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                   "ResourceManager::getOrCreate: PHYSFSRWOPS_openRead(\"%s\") "
                   "failed: '%s'\n",
                   path, SDL_GetError());
    }
  }
  const StrongRes &strong = StrongRes(factory(ops));
  weak = strong;
  return strong;
}
}  // namespace renity
