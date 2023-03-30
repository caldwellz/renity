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

// For RENITY_USE_STL
#include "config.h"

// Common types and SDL's C language support
#include <SDL3/SDL_stdinc.h>

#ifdef __cplusplus
#ifdef RENITY_USE_STL
// PORTABILITY NOTE: Some features require RTTI and at least C++11
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace renity {
template <class Base, class Derived>
constexpr void requireBaseOf() {
  static_assert(std::is_base_of<Base, Derived>::value,
                "Type is not derived from the required base class.");
}
/*
template <class T, class U>
using dynamicPointerCast = std::dynamic_pointer_cast<T, U>;
template <class T, class U>
using staticPointerCast = std::static_pointer_cast<T, U>;
template <class T, class U>
using makeSharedPtr = std::make_shared<T, U>;
template <typename T>
using typeHash = typeid(T).hash_code;
*/
// using declarations don't work with these for some reason...
#define dynamicPointerCast std::dynamic_pointer_cast
#define staticPointerCast std::static_pointer_cast
#define makeSharedPtr std::make_shared
#define typeHash(T) typeid(T).hash_code()
#define FuncPtr std::function

template <class T, class D = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, D>;
template <typename T>
using SharedPtr = std::shared_ptr<T>;
template <typename T>
using WeakPtr = std::weak_ptr<T>;
template <typename T>
using Vector = std::vector<T>;

using String = std::string;
}  // namespace renity
#else
#error \
    "It appears you're not using the C++ STL. Please replace features here as needed."
#endif  // RENITY_HAVE_STL
#endif  // __cplusplus
#endif  // RENITY_TYPES_H_
