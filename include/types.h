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
#if defined RENITY_USE_STL && __cplusplus >= 201703L
// PORTABILITY NOTE: Some features require RTTI and at least C++17
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <variant>
#include <vector>

namespace renity {
// Generic identifier types
using Id = Uint32;
using ActionCategoryId = Id;
using ActionId = Id;
using ChunkId = Id;
using EntityId = Uint64;
using TileId = Id;
using Timestamp = Uint64;

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
// Common macros
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((b) < (a)) ? (a) : (b))

// using declarations don't work with these for some reason...
#define constStrlen(str) std::char_traits<char>::length(str)
#define dynamicPointerCast std::dynamic_pointer_cast
#define staticPointerCast std::static_pointer_cast
#define makeSharedPtr std::make_shared
#define toString(x) std::to_string(x)
#define typeHash(T) typeid(T).hash_code()
#define typeName(T) typeid(decltype(T)).name()
#define FuncPtr std::function
// TODO: Convert to custom func if we ever need to support numerical conversions
#define getAs std::get

template <class T, class D = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, D>;
template <typename T>
using SharedPtr = std::shared_ptr<T>;
template <typename T>
using WeakPtr = std::weak_ptr<T>;
template <typename T>
using Vector = std::vector<T>;

using AtomicFlag8 = std::atomic<uint_fast8_t>;
using String = std::string;
using PrimitiveVariant =
    std::variant<String, Uint64, Uint32, Uint16, Uint8, Sint64, Sint32, Sint16,
                 Sint8, double, float, void*>;
}  // namespace renity
#else
#error \
    "It appears you're not using at least a C++17 STL. Please upgrade your toolchain or replace features here as needed."
#endif  // RENITY_HAVE_STL
#endif  // __cplusplus
#endif  // RENITY_TYPES_H_
