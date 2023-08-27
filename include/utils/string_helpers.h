/****************************************************
 * string_helpers.h: Various String utilities       *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>

#include <algorithm>
#include <cctype>

#include "types.h"

namespace renity {
// The std functions purposely do int<->char conversions - shush MSVC warnings
#pragma warning(disable : 4244)

/** Converts a string (non-multibyte) to uppercase, and returns a new String. */
inline String toUpper(String str) {
  std::transform(str.begin(), str.end(), str.begin(), std::toupper);
  return str;
}

/** Converts a string (non-multibyte) to lowercase, and returns a new String. */
inline String toLower(String str) {
  std::transform(str.begin(), str.end(), str.begin(), std::tolower);
  return str;
}

/** Does a case-sensitive comparison between prefix and the beginning of str. */
inline bool beginsWith(const String str, const String prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}

/** Does a case-sensitive comparison between suffix and the end of str. */
inline bool endsWith(const String str, const String suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/** Parses an ARGB #color hex string into an RBGA value. */
inline Uint32 strToColor(const String str) {
  Uint32 rgb;
  Uint8 a;
  String slicedStr = beginsWith(str, "#") ? str.substr(1) : str;
  if (slicedStr.length() > 8 || slicedStr.length() < 6) return 0;
  if (slicedStr.length() == 8) {
    rgb = std::stol(slicedStr.substr(2), nullptr, 16);
    a = std::stol(slicedStr.substr(0, 2), nullptr, 16);
  } else {
    rgb = std::stol(slicedStr, nullptr, 16);
    a = 0xFF;
  }
  return (rgb << 8) | a;
}
#pragma warning(default : 4244)
}  // namespace renity
