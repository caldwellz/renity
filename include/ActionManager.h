/****************************************************
 * ActionManager.h: Action handler pub-sub          *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "ActionHandler.h"
#include "types.h"

namespace renity {
class Action;

class RENITY_API ActionManager {
 public:
  ActionManager();
  ~ActionManager();

  /* TODO: Someday it may make sense to allow copying/moving ActionManager
   * objects, but for now, delete the functions to prevent it.
   */
  ActionManager(ActionManager &other) = delete;
  ActionManager(const ActionManager &other) = delete;
  ActionManager &operator=(ActionManager &other) = delete;
  ActionManager &operator=(const ActionManager &other) = delete;

  /** Get the active (current) ActionManager.
   * \returns A pointer to the last-activated ActionManager, or null if none
   * are valid.
   */
  static ActionManager *getActive();

  /** Get the name of a registered action or category from its id.
   * \returns A String containing the name that translates to the given id,
   * or an empty String if nothing has been registered with that name.
   */
  String getNameFromId(Id id) const;

  /** Activate this ActionManager.
   * Makes it the "current" manager for subsequent posts by other managers.
   */
  void activate();

  /** Post a new Action to the handler queue.
   * This is safe to call from any thread, including from handler callbacks.
   * \param action The Action to be handled.
   * Category handlers will run in registration order, but often simultaneously.
   * \returns True if the actionId is registered to a category that has at least
   * one handler registered; false otherwise.
   */
  bool post(Action action);

  /** Add an ActionHandler to a given category.
   * Note that all side effects of the handler *must* be thread safe, since it
   * may be called from several different worker threads *simultaneously*.
   * \returns The id of the category name, to make static assignment easier.
   */
  ActionCategoryId subscribe(ActionHandlerPtr handler, String actionCategory);

  /** (Re)assign an ActionId to a category.
   * \returns The id of the action name, to make static assignment easier.
   */
  ActionId assignCategory(String actionName, String actionCategory);

  // TODO: Do we need these?
  /** Continue enqueuing Actions, but stop processing them. */
  // void pause();

  /** Resume processing Actions. */
  // void resume();

 private:
  struct Impl;
  Impl *pimpl_;
};
}  // namespace renity
