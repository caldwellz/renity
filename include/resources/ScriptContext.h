/****************************************************
 * ScriptContext.h: JavaScript execution context    *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "3rdparty/duktape/duktape.h"
#include "ActionHandler.h"
#include "Resource.h"
#include "types.h"

namespace renity {
class RENITY_API ScriptContext : public Resource, public ActionHandler {
 public:
  ScriptContext();
  ~ScriptContext();

  /** Check whether the environment setup and init script were successful. */
  bool initialized();

  /** Evaluate a script file.
   * \param path A script file path, in PhysFS notation.
   * \returns False if the script failed to compile or returned a boolean false,
   * true otherwise.
   */
  bool evalFile(String path);

  /** Add a native function to a global context path.
   * \param path The object path to add the function to, e.g. "Engine.myFunc"
   * \param func The native function callback.
   * \param nargs The number of arguments the function expects.
   */
  void registerFunc(String path, duk_c_function func, duk_idx_t nargs);

  void load(SDL_RWops* src);
  void handleAction(const ActionCategoryId categoryId, const Action* action);

 private:
  struct Impl;
  Impl* pimpl_;
};
using ScriptContextPtr = SharedPtr<ScriptContext>;
}  // namespace renity
