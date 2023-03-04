/****************************************************
 * types.h: Centralized basic types                 *
 * Copyright (C) 2021-2023 Zach Caldwell            *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_TYPES_H_
#define RENITY_TYPES_H_

// Common types and SDL's C language support
#include <SDL3/SDL_stdinc.h>

#ifdef __cplusplus
// For RENITY_USE_STL
#include "config.h"
#ifdef RENITY_USE_STL
#include <string>
#include <vector>

namespace renity {
using String = std::string;
template <typename T>
using Vector = std::vector<T>;
}  // namespace renity
#else
// TODO: Find replacements for platforms with no STL
#error \
    "It appears you're not using the C++ STL. Please specify a replacement String and Vector."
#endif  // RENITY_HAVE_STL
#endif  // __cplusplus
#endif  // RENITY_TYPES_H_
