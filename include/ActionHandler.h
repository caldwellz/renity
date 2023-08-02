/****************************************************
 * ActionHandler.h: Listen for and handle Actions   *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Action.h"
#include "types.h"

namespace renity {
class RENITY_API ActionHandler {
 public:
  ActionHandler(){};
  virtual ~ActionHandler(){};

  /** Handle a new Action in a given category.
   * Invoked by the ActionManager for new Actions in categories this handler is
   * registered under. Note that all side effects of this function *must* be
   * thread safe, since *any* worker thread may call it *simultaneously*.
   * @param categoryId A category this handler was registered under.
   * @param action The Action to process.
   */
  virtual void handleAction(const ActionCategoryId categoryId,
                            const Action *action) = 0;
};
using ActionHandlerPtr = SharedPtr<ActionHandler>;
}  // namespace renity
