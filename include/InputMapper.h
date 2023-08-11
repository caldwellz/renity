/****************************************************
 * InputMapper.h: Convert user input to Actions     *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include <SDL3/SDL_events.h>

#include "ActionHandler.h"
#include "types.h"

namespace renity {
class RENITY_API InputMapper {
 public:
  InputMapper(const char *loadPath = nullptr);
  ~InputMapper();

  /* TODO: Someday it may make sense to allow copying/moving InputMapper
   * objects, but for now, delete the functions to prevent it.
   */
  InputMapper(InputMapper &other) = delete;
  InputMapper(const InputMapper &other) = delete;
  InputMapper &operator=(InputMapper &other) = delete;
  InputMapper &operator=(const InputMapper &other) = delete;

  void load(const char *path);
  bool save(const char *path);

 private:
  struct Impl;
  Impl *pimpl_;
  ActionHandlerPtr pimplHolder_;
  friend int inputEventProcessor(void *userdata, SDL_Event *event);
};
}  // namespace renity
