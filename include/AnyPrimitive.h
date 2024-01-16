/****************************************************
 * AnyPrimitive.h: Store any primitive (or String)  *
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
struct RENITY_API AnyPrimitive {
  AnyPrimitive();
  AnyPrimitive(String s);
  AnyPrimitive(void* ptr0, void* ptr1 = nullptr, void* ptr2 = nullptr,
               void* ptr3 = nullptr);
  AnyPrimitive(Uint64 u0, Uint64 u1 = 0, Uint64 u2 = 0, Uint64 u3 = 0);
  AnyPrimitive(Sint64 s0, Sint64 s1 = 0, Sint64 s2 = 0, Sint64 s3 = 0);
  AnyPrimitive(double d0, double d1 = 0, double d2 = 0, double d3 = 0);

  // Fill in the rest of the data item manually if needed
  AnyPrimitive(Uint32 u0, Uint32 u1 = 0, Uint32 u2 = 0, Uint32 u3 = 0);
  AnyPrimitive(Sint32 s0, Sint32 s1 = 0, Sint32 s2 = 0, Sint32 s3 = 0);
  AnyPrimitive(float f0, float f1 = 0, float f2 = 0, float f3 = 0);
  AnyPrimitive(Uint16 u0, Uint16 u1 = 0, Uint16 u2 = 0, Uint16 u3 = 0);
  AnyPrimitive(Sint16 s0, Sint16 s1 = 0, Sint16 s2 = 0, Sint16 s3 = 0);
  AnyPrimitive(Uint8 u0, Uint8 u1 = 0, Uint8 u2 = 0, Uint8 u3 = 0);
  AnyPrimitive(Sint8 s0, Sint8 s1 = 0, Sint8 s2 = 0, Sint8 s3 = 0);

  union data {
    // Strings have >= 32 *bytes* on common platforms, so make use of the space
    String str;
    void* ptr[4];
    Uint64 u64[4];
    Sint64 s64[4];
    double d64[4];
    Uint32 u32[8];
    Sint32 s32[8];
    float f32[8];
    Uint16 u16[16];
    Sint16 s16[16];
    Uint8 u8[32];
    Sint8 s8[32];
  };

  enum DataType {
    T_NONE = 0,
    T_STRING,
    T_PTR,
    T_U64,
    T_S64,
    T_D64,
    T_U32,
    T_S32,
    T_F32,
    T_U16,
    T_S16,
    T_U8,
    T_S8,
  };
  DataType type;
};
}  // namespace renity
