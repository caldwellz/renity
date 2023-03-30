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

  /** Select a relative path into the Dictionary.
   * Further operations, e.g. get() and put(), will use this as a prefix.
   * @param path Selection path, relative to the current selection.
   * @param autoCreate Whether to create any leading keys that don't yet exist.
   * @return How much the selection depth increased by, based on the number of
   * path segments. 0 if any segments were missing and autoCreate = false.
   */
  inline size_t select(const char *path, bool autoCreate = false) {
    return select(path, autoCreate, true);
  }

  /** Select a numerical index from the current path.
   * Further operations, e.g. get() and put(), will use this as a prefix.
   * @param index Selection path, relative to the current selection.
   * @param autoCreate Create the index if it doesn't yet exist.
   * @return 1 if the index existed or was created, 0 otherwise.
   */
  size_t selectIndex(Uint32 index, bool autoCreate = false);

  /** Unwind prior selects, up to the root level of the Dictionary.
   * @param depth Selection depth to unwind. Default is back to root level.
   */
  void unwind(size_t depth = SIZE_MAX);

  /** Get the beginning index of the given array.
   * @param key What to enumerate. Defaults to the current selection.
   * @return The beginning index, or UINT32_MAX if the indices aren't
   * enumerable.
   */
  Uint32 begin(const char *key = nullptr);

  /** Get the one-past-the-end index of the given array.
   * @param key What to enumerate. Defaults to the current selection.
   * @return The ending index, or UINT32_MAX if the indices aren't enumerable.
   */
  Uint32 end(const char *key = nullptr);

  /** Enumerate an object or array using a callback.
   * @param path A path to enumerate.
   * Can be a nullptr to use the currently-selected path.
   * @param callback Called for each enumeration with the key as a string.
   * It should return true to continue enumerating, or false otherwise. Any
   * select()s done in-callback are automatically unwound after each invocation.
   * @return The number of properties enumerated, if any.
   */
  Uint32 enumerate(const char *path,
                   const FuncPtr<bool(Dictionary &, const String &)> &callback);

  /** Enumerate an array using a callback.
   * @param path A path to enumerate.
   * Can be a nullptr to use the currently-selected path.
   * @param callback Called for each enumeration with the index as a unsigned
   * int. It should return true to continue enumerating, or false otherwise. Any
   * select()s done in-callback are automatically unwound after each invocation.
   * @return The number of properties enumerated, if any.
   */
  Uint32 enumerateArray(
      const char *path,
      const FuncPtr<bool(Dictionary &, const Uint32 &)> &callback);

  /** Get a Property value of the given type if it exists.
   * @param key A key to attempt to get the value of.
   * Can be a nullptr to use the currently-selected path.
   * @param valOut Where to store the value, if found. Can be a nullptr.
   * @return True if the value exists, false otherwise.
   */
  template <typename T>
  bool get(const char *key, T *valOut);

  /** Get an index value of the given type if it exists.
   * @param index An index into the currently-selected path to get the value of.
   * @param valOut Where to store the value, if found. Can be a nullptr.
   * @return True if the value exists, false otherwise.
   */
  template <typename T>
  bool getIndex(Uint32 index, T *valOut);

  /** Create an array at a given path.
   * @param key The key to store the array under.
   * @return True if the array was able to be stored (or the path was already an
   * array), false otherwise.
   */
  bool putArray(const char *key);

  /** Create a key-value subobject at a given path.
   * @param key The key to store the object under.
   * @return True if the object was able to be stored (or the path was already
   * an object), false otherwise.
   */
  // bool putObject(const char *key);

  /** Store a value of the given type at the end of the selected array.
   * @param val The value to append.
   * @return True if the value was able to be appended, false otherwise.
   */
  template <typename T>
  bool push(T val);

  /** Store a Property value of the given type.
   * @param key The key to store the value under.
   * @param val The value to store. Will overwrite a previous value of any type.
   * @return True if the value was able to be stored, false otherwise.
   */
  template <typename T>
  bool put(const char *key, T val);

  /** Store an index value of the given type.
   * @param index An index into the currently-selected path to set the value of.
   * @param val The value to store. Will overwrite a previous value of any type.
   * @return True if the value was able to be stored, false otherwise.
   */
  template <typename T>
  bool putIndex(Uint32 index, T val);

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

  /** Get a Property value, or a default value, of the given type.
   * @param index An index to attempt to get the value of.
   * @param defaultVal Default value to store/return if the index was not found
   * or contained a different data type.
   * @return Either the stored value or the default value, as appropriate.
   */
  template <typename T>
  T keepIndex(Uint32 index, T defaultVal) {
    T storedVal;
    if (getIndex<T>(index, &storedVal)) return storedVal;
    putIndex<T>(index, defaultVal);
    return defaultVal;
  }

 protected:
  size_t select(const char *path, bool autoCreate, bool loadValue);

 private:
  struct Impl;
  Impl *pimpl_;
};
using DictionaryPtr = SharedPtr<Dictionary>;
}  // namespace renity
