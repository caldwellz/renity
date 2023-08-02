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
#include "utils/id_helpers.h"

namespace renity {
/** Generic single-key hash table. Currently just glosses over a
 * std::unordered_map, but will eventually use a custom implementation. */
template <typename Key, typename Val>
class HashTable {
 public:
  /** Check if an item exists in the table */
  bool exists(Key k) { return map.find(getId(k)) != map.end(); }

  /** Get an item from the table, constructing a new one if it doesn't exist. */
  Val &get(Key k) {
#ifdef RENITY_DEBUG_VERBOSE
    cout << "HashTable::get '" << k << "' (key:";
    if (std::is_same<Key, const char *>::value) {
      for (char *ptr = (char *)k; *ptr; ++ptr) {
        cout << std::hex << (Uint16)*ptr;
      }
    } else {
      cout << "[not a C str]";
    }
    cout << ", hash:" << std::hex << getId(k) << ")" << endl;
#endif
    return map[getId(k)];
  }

  /** Insert or overwrite an item in the table. */
  void put(Key k, Val v) { map[getId(k)] = v; }

  /** Remove an item from the table. */
  void erase(Key k) { map.erase(getId(k)); }

  /** Remove *all* items from the table. */
  void clear() { map.clear(); }

 private:
  unordered_map<Id, Val> map;
};

/** Dual-key hash table that currently just mashes two Id's together. */
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
    Uint64 valA = (Uint64)getId(a);
    Uint64 valB = (Uint64)getId(b);
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
