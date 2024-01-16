/****************************************************
 * EntityManager.cc: Entity Component System (ECS)  *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "EntityManager.h"

#include <SDL3/SDL_log.h>

#include "3rdparty/duktape/duktape.h"
#include "ResourceManager.h"
#include "utils/rwops_utils.h"

namespace renity {
EntityManager *currentEntityManager = nullptr;
struct EntityManager::Impl {
  Impl() {
    // Hold a shared dictionary pointer for the lifetime of the EntityManager so
    // it doesn't get auto-deleted before then, since resources are temporary.
    dict = ResourceManager::getActive()->get<Dictionary>("<shared-dict>");

    // TODO: Load previous max EntityId from somewhere (chunk 0?)
    nextChunk = 1;
    nextEntity = 1;
  }
  ~Impl() {}

  DictionaryPtr dict;
  ChunkId nextChunk;
  EntityId nextEntity;
};

RENITY_API EntityManager::EntityManager() {
  pimpl_ = new Impl();
  if (!currentEntityManager) currentEntityManager = this;
}

RENITY_API EntityManager::~EntityManager() {
  if (currentEntityManager == this) {
    currentEntityManager = nullptr;
  }
  delete this->pimpl_;
}

RENITY_API void EntityManager::activate() { currentEntityManager = this; }

RENITY_API EntityManager *EntityManager::getActive() {
  return currentEntityManager;
}

RENITY_API DictionaryPtr EntityManager::getDictionary() const {
  return pimpl_->dict;
}

/** Load a world chunk from "[pathPrefix]/[id].chunk".
 * @param id A ChunkId to load.
 * @param pathPrefix An optional prefix to add to the PhysFS path.
 * @return True if the chunk was loaded successfully, false otherwise.
 */
RENITY_API bool EntityManager::loadChunk(const ChunkId id,
                                         const char *pathPrefix) {
  String path(pathPrefix ? pathPrefix : "");
  path += "/" + toString(id) + ".chunk";
  DictionaryPtr chunk =
      ResourceManager::getActive()->get<Dictionary>(path.c_str());
  // TODO: Merge chunk into shared dict
  return true;
}

/** Save a world chunk to "[pathPrefix]/[id].chunk".
 * @param id A ChunkId to save.
 * @param pathPrefix An optional prefix to add to the PhysFS path.
 * @return True if the chunk was saved successfully, false otherwise.
 */
RENITY_API bool EntityManager::saveChunk(const ChunkId id,
                                         const char *pathPrefix) {
  String path(pathPrefix ? pathPrefix : "");
  path += "/" + toString(id) + ".chunk";
  // TODO: Select path and save it
  return true;
}

/** Remove a world chunk and all entities within it from the cache.
 * @param id A ChunkId to unload.
 */
RENITY_API void EntityManager::unloadChunk(const ChunkId id) {
  pimpl_->dict->unwind();
  pimpl_->dict->enumerateArray("WorldChunk",
                               [&](Dictionary &dict, const EntityId &eId) {
                                 static ChunkId eCid = 0;
                                 if (dict.get(nullptr, &eCid) && eCid == id) {
                                   destroyEntity(eId);
                                 }
                                 return true;
                               });
}

/** Set up a new Entity using an optional blueprint file and chunk assignment.
 * @param chunk A ChunkId to assign the Entity to.
 * @param blueprint An optional filename, in platform-independent notation.
 * @return The EntityId of the new Entity.
 */
RENITY_API EntityId EntityManager::createEntity(const ChunkId chunk,
                                                const char *blueprint) {
  EntityId id = pimpl_->nextEntity++;
  // TODO: Merge blueprint and then set WorldChunk
  return id;
}

/** Destroy an Entity, removing all of its components.
 * @param e The EntityId to destroy.
 */
RENITY_API void EntityManager::destroyEntity(EntityId e) {
  pimpl_->dict->unwind();
  pimpl_->dict->enumerate(nullptr, [&](Dictionary &dict, const String &key) {
    // TODO: Implement dict.eraseIndex() and test; does modifying the object
    // during unloadChunk work? Not very efficient.
    // Just use chunks as top-level dict indexes and search them for entities?
    // dict.eraseIndex(e);
    return true;
  });
}
}  // namespace renity
