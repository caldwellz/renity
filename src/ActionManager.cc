/****************************************************
 * ActionManager.cc: Action handler pub-sub         *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "ActionManager.h"

#include <SDL3/SDL_log.h>

#include "Action.h"
#include "ActionHandler.h"
#include "HashTable.h"

namespace renity {
ActionManager* currentActionManager = nullptr;
static const Uint8 MAX_THREAD_USAGE = 9;

struct ActionManager::Impl {
  Impl() {}
  ~Impl() {}

  HashTable<ActionId, ActionCategoryId> categories;
  HashTable<ActionCategoryId, Vector<SharedPtr<ActionHandler> > > handlers;
};

RENITY_API ActionManager::ActionManager() {
  pimpl_ = new Impl();
  activate();

  // How many workers can we spin up, excluding the main (rendering) thread?
  const Uint32 hwThreads = std::thread::hardware_concurrency();
  const Uint32 workerThreads = MIN(MAX_THREAD_USAGE, MAX(hwThreads, 2)) - 1;
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
              "ActionManager: Using %u of %u hardware threads for workers.",
              workerThreads, hwThreads);
}

RENITY_API ActionManager::~ActionManager() {
  if (currentActionManager == this) currentActionManager = nullptr;
  delete this->pimpl_;
}

RENITY_API ActionManager* ActionManager::getActive() {
  if (!currentActionManager) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "ActionManager::getActive: No active instance.");
  }
  return currentActionManager;
}

RENITY_API void ActionManager::activate() { currentActionManager = this; }

RENITY_API bool ActionManager::post(Action action) {
  if (!pimpl_->categories.exists(action.getId())) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "ActionManager::post: ActionId 0x%04x has no "
                "registered category.",
                action.getId());
    return false;
  }

  ActionCategoryId catId = pimpl_->categories.get(action.getId());
  if (pimpl_->handlers.get(catId).empty()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "ActionManager::post: ActionCategoryId 0x%04x has no "
                "subscribed handlers - ignoring ActionId 0x%04x.",
                catId, action.getId());
    return false;
  }

  // TODO: Actually implement multithreaded workers when/if the basic
  // architecture becomes a widely-used performance bottleneck. For now,
  // we're fully synchronous to delay potential bugs and ease implementation.
  for (auto handlerPtr : pimpl_->handlers.get(catId)) {
    handlerPtr->handleAction(catId, &action);
  }

  return true;
}

RENITY_API void ActionManager::subscribe(SharedPtr<ActionHandler> handler,
                                         ActionCategoryId actionCategory) {
  if (!handler) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "ActionManager::subscribe: No handler given for actionCategory %u.",
        actionCategory);
    return;
  }

  pimpl_->handlers.get(actionCategory).push_back(handler);
  SDL_LogDebug(
      SDL_LOG_CATEGORY_APPLICATION,
      "ActionManager::subscribe: Subscribed new handler for category 0x%08x.",
      actionCategory);
}

RENITY_API ActionId ActionManager::assignCategory(
    ActionId actionId, ActionCategoryId actionCategory) {
  pimpl_->categories.put(actionId, actionCategory);
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "ActionManager::assignCategory: Assigned action 0x%08x to "
               "category 0x%08x.",
               actionId, actionCategory);
  return actionId;
}
}  // namespace renity
