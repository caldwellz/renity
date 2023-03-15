/****************************************************
 * Dictionary.h: Configuration resource class       *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "Resource.h"
#include "types.h"

namespace renity {
/** Encapsulates a JSON object containing key:value pairs. */
class RENITY_API Dictionary : public Resource {
 public:
  Dictionary();
  ~Dictionary();

  void load(SDL_RWops *src);

  /** Save the Dictionary contents to a JSON file.
   * @param destPath Destination file path. Contents will always be written as
   * JSON, regardless of file extension.
   * @return True if the operation succeeded, false otherwise.
   */
  bool saveJSON(const char *destPath);

  /** Save the Dictionary contents to a CBOR file.
   * @param destPath Destination file path. Contents will always be written as
   * CBOR, regardless of file extension.
   * @return True if the operation succeeded, false otherwise.
   */
  bool saveCBOR(const char *destPath);

  /** Get a Property value of the given type if it exists.
   * @param key A key to attempt to get the value of.
   * @param valOut Where to store the value, if found. Can be a nullptr.
   * @return True if the value exists, false otherwise.
   */
  template <typename T>
  bool get(const char *key, T *valOut);

  /** Store a Property value of the given type.
   * @param key The key to store the value under.
   * @param val The value to store. Will overwrite a previous value of any type.
   * @return True if the value was able to be stored, false otherwise.
   */
  template <typename T>
  bool put(const char *key, T val);

  /** Get a Property value, or a default value, of the given type.
   * @param key A key to attempt to get the value of.
   * @param defaultVal Default value to store/return if the key was not found
   * or contained a different data type.
   * @return Either the stored value or the default value, as appropriate.
   */
  template <typename T>
  T keep(const char *key, T defaultVal) {
    T storedVal;
    if (get<T>(key, &storedVal)) return storedVal;
    put<T>(key, defaultVal);
    return defaultVal;
  }

 private:
  struct Impl;
  Impl *pimpl_;
  bool traverseObj(const char *rawKey, bool autoCreate);
  void unwindObj();
};
using DictionaryPtr = SharedPtr<Dictionary>;
}  // namespace renity
