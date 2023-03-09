/****************************************************
 * HashTable.h: Custom hash table(s)                *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#ifdef RENITY_DEBUG_VERBOSE
#include <iostream>
#endif
#include <unordered_map>
using namespace std;

#include "types.h"

namespace renity {
/** Generic single-key hash table. Currently just glosses over a
 * std::unordered_map, but will eventually use a custom implementation. */
template <typename Key, typename Val>
class HashTable {
 public:
  /** Get an item from the table, constructing a new one if it doesn't exist. */
  Val &get(Key &k) {
#ifdef RENITY_DEBUG_VERBOSE
    cout << "HashTable::get '" << k << "' (key:";
    if (std::is_same<Key, const char *>::value) {
      for (char *ptr = (char *)k; *ptr; ++ptr) {
        cout << std::hex << (Uint16)*ptr;
      }
    } else {
      cout << "[not a C str]";
    }
    cout << ", hash:" << std::hex << mapHash(k) << ")" << endl;
#endif
    return map[mapHash(k)];
  }

  /** Insert or overwrite an item in the table. */
  void put(Key &k, Val &v) { map[mapHash(k)] = v; }

  /** Remove an item from the table. */
  void erase(Key &k) { map.erase(mapHash(k)); }

  /** Remove *all* items from the table. */
  void clear() { map.clear(); }

 private:
  size_t mapHash(Key &k) {
    // Have to convert C strings to C++ strings, otherwise it'll hash the ptr
    if (std::is_same<Key, const char *>::value) {
      String s(k);
      return std::hash<String>{}(s);
    }
    return std::hash<Key>{}(k);
  }
  unordered_map<size_t, Val> map;
};

/** Dual-key hash table. Currently just mashes two std::hash outputs together,
 * but will eventually use XXH3 with a uint-optimized table. */
template <typename KeyA, typename KeyB, typename Val>
class DualHashTable {
 public:
  /** Get an item from the table, constructing a new one if it doesn't exist. */
  Val &get(KeyA &a, KeyB &b) {
    Uint64 hash = dualHash(a, b);
    return map.get(hash);
  }

  /** Insert or overwrite an item in the table. */
  void put(KeyA &a, KeyB &b, Val &v) { map.put(dualHash(a, b), v); }

  /** Remove an item from the table. */
  void erase(KeyA &a, KeyB &b) { map.erase(dualHash(a, b)); }

  /** Remove *all* items from the table. */
  void clear() { map.clear(); }

 protected:
  Uint64 dualHash(KeyA &a, KeyB &b) {
    Uint64 valA = std::hash<KeyA>{}(a);
    Uint32 valB = std::hash<KeyB>{}(b);
    Uint64 res = valA << 32 | valB;
#ifdef RENITY_DEBUG_VERBOSE
    cout << "dualHash: A='" << a << "' (" << std::hex << valA << "), B='" << b
         << "' (" << std::hex << valB << "), Res=" << std::hex << res << endl;
#endif
    return res;
  }

 private:
  HashTable<Uint64, Val> map;
};
}  // namespace renity
