/****************************************************
 * Action.h: Action event class                     *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "types.h"

namespace renity {
class ActionManager;
/*
enum ActionStatus {
  AS_Waiting = 0x0,
  AS_Processing = 0x1,
  AS_Complete = 0x2,
  AS_Modifying = 0x4
};
*/
class RENITY_API Action {
 public:
  Action();
  Action(const ActionId id, Vector<PrimitiveVariant> data);
  Action(const String actionName, Vector<PrimitiveVariant> data);

  ActionId getId() const;

  Timestamp getCreatedAt() const;

  // TODO: Determine if we need this once workers are actually multithreaded
  // ActionStatus getStatus() const;

  PrimitiveVariant getData(const size_t index) const;

  size_t getDataCount() const;

  template <class T>
  T getDataAs(size_t index) const {
    return getAs<T>(getData(index));
  }

 protected:
  friend class ActionManager;
  // AtomicFlag8 status_;
  ActionId id_;
  Timestamp createdAt_;
  Vector<PrimitiveVariant> data_;
};
}  // namespace renity
