/****************************************************
 * Action.cc: Action event class                    *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Action.h"

#include <SDL3/SDL_timer.h>

#include "ActionManager.h"
#include "types.h"
#include "utils/id_helpers.h"

namespace renity {
RENITY_API Action::Action(const ActionId id, Vector<PrimitiveVariant> data) {
  // status_.store(AS_Waiting);
  id_ = id;
  createdAt_ = SDL_GetTicks();
  data_ = data;
}

RENITY_API Action::Action(const String actionName,
                          Vector<PrimitiveVariant> data) {
  Action(renity::getId(actionName), data);
}

RENITY_API ActionId Action::getId() const { return id_; }

RENITY_API String Action::getName() const {
  return ActionManager::getActive()->getNameFromId(id_);
}

RENITY_API Timestamp Action::getCreatedAt() const { return createdAt_; }
/*
RENITY_API ActionStatus Action::getStatus() const {
  const uint_fast8_t stat = status_.load(std::memory_order_relaxed);
  if (stat & AS_Modifying) return AS_Modifying;
  if (stat & AS_Complete) return AS_Complete;
  if (stat & AS_Processing) return AS_Processing;
  return AS_Waiting;
}
*/
RENITY_API PrimitiveVariant Action::getData(const size_t index) const {
  return data_.at(index);
}

RENITY_API size_t Action::getDataCount() const { return data_.size(); }
}  // namespace renity
