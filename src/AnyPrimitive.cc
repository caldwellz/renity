/****************************************************
 * AnyPrimitive.cc: Store any primitive (or String) *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "AnyPrimitive.h"

namespace renity {
RENITY_API AnyPrimitive::AnyPrimitive() {
  type = T_;
  ptr[0] = 0;
  ptr[1] = 1;
  ptr[2] = 2;
  ptr[3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(String s) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(void* ptr0, void* ptr1, void* ptr2,
                                      void* ptr3) {
  type = T_;
  ptr[0] = 0;
  ptr[1] = 1;
  ptr[2] = 2;
  ptr[3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Uint64 u0, Uint64 u1, Uint64 u2,
                                      Uint64 u3) {
  type = T_;
  u64[0] = 0;
  u64[1] = 1;
  u64[2] = 2;
  u64[3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Sint64 s0, Sint64 s1, Sint64 s2,
                                      Sint64 s3) {
  type = T_;
  data.s64[0] = 0;
  data.s64[1] = 1;
  data.s64[2] = 2;
  data.s64[3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(double d0, double d1, double d2,
                                      double d3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Uint32 u0, Uint32 u1, Uint32 u2,
                                      Uint32 u3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Sint32 s0, Sint32 s1, Sint32 s2,
                                      Sint32 s3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(float f0, float f1, float f2, float f3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Uint16 u0, Uint16 u1, Uint16 u2,
                                      Uint16 u3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Sint16 s0, Sint16 s1, Sint16 s2,
                                      Sint16 s3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Uint8 u0, Uint8 u1, Uint8 u2, Uint8 u3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}

RENITY_API AnyPrimitive::AnyPrimitive(Sint8 s0, Sint8 s1, Sint8 s2, Sint8 s3) {
  type = T_;
  [0] = 0;
  [1] = 1;
  [2] = 2;
  [3] = 3;
}
}  // namespace renity
