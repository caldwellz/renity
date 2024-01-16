/****************************************************
 * EntityManager.h: Entity Component System (ECS)   *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Dictionary.h"
#include "types.h"

namespace renity {
constexpr ChunkId CURRENT_CHUNK = 0;

class RENITY_API EntityManager {
 public:
  EntityManager();
  ~EntityManager();

  /* TODO: Someday it may make sense to allow copying/moving EntityManager
   * objects, but for now, delete the functions to prevent it.
   */
  EntityManager(EntityManager &other) = delete;
  EntityManager(const EntityManager &other) = delete;
  EntityManager &operator=(EntityManager &other) = delete;
  EntityManager &operator=(const EntityManager &other) = delete;

  /** Activate this EntityManager.
   * Makes it the "current" manager for any subsequent entity operations.
   */
  void activate();

  /** Get the active (current) EntityManager.
   * \returns A pointer to the last-activated EntityManager, or null if none
   * are valid.
   */
  static EntityManager *getActive();

  /** Load a world chunk from "[pathPrefix]/chunks/[id].chunk".
   * @param id A ChunkId to load.
   * @param pathPrefix An optional prefix to add to the PhysFS path.
   * @return True if the chunk was loaded successfully, false otherwise.
   */
  bool loadChunk(const ChunkId id, const char *pathPrefix = nullptr);

  /** Save a world chunk to "[pathPrefix]/chunks/[id].chunk".
   * @param id A ChunkId to save.
   * @param pathPrefix An optional prefix to add to the PhysFS path.
   * @return True if the chunk was saved successfully, false otherwise.
   */
  bool saveChunk(const ChunkId id, const char *pathPrefix = nullptr);

  /** Unload a world chunk and all entities within it from the cache.
   * Frees memory, but does not "destroy" the entities; their Ids stay reserved.
   * @param id A ChunkId to unload.
   */
  void unloadChunk(const ChunkId id);

  /** Set up a new Entity using a blueprint file and optional chunk assignment.
   * @param blueprint An entity blueprint name.
   * @param chunk An optional ChunkId to assign the Entity to.
   * @return The EntityId of the new Entity.
   */
  EntityId createEntity(const String blueprint,
                        const ChunkId chunk = CURRENT_CHUNK);

  /** Destroy an Entity, enabling its Id to be reused.
   * @param e The EntityId to destroy.
   */
  void destroyEntity(EntityId e);

 protected:
  /** Get the Dictionary shared by the entity components in this manager.
   * \returns A pointer to the component Dictionary.
   */
  DictionaryPtr getDictionary() const;

 private:
  struct Impl;
  Impl *pimpl_;
};
}  // namespace renity
