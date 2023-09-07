/****************************************************
 * Test - Dictionary                                *
 * Copyright (C) 2023 Zach Caldwell                 *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Dictionary.h"

#include "Application.h"
#include "ResourceManager.h"
using namespace renity;

#include <SDL3/SDL.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  Application app(argc, argv);
  assert(app.initialize(true));

  // Attempt to load CBOR file and then modify the result
  DictionaryPtr config =
      ResourceManager::getActive()->get<Dictionary>("config.cbor");
  assert(config->put<bool>("stuff", true));
  SDL_Log("config.cbor[foo]: %s\n", config->keep<const char *>("foo", "bar"));
  assert(config->push<const char *>("valA"));  // ["0"] -> object length==0
  assert(config->put<float>("thing", 42.1234));
  config->keep<const char *>("bar.woof.foo", "xyzzy");
  config->keep<const char *>("bar.woof.otherThing", "abcd");
  assert(config->push<const char *>("valB"));  // ["0"] -> object length==0
  assert(config->putArray("bar.woof.baz"));
  assert(config->select("bar.woof.baz") == 3);
  assert(config->putAt<const char *>(3, "ab"));  // [3] -> length==4
  assert(config->push<const char *>("cd"));         // [4] -> length==5
  assert(config->putAt<const char *>(0, "ef"));  // [0] -> length==5
  assert(config->push<const char *>("gh"));         // [5] -> length==6
  assert(config->selectIndex(7, true) == 1);        // [7] -> length==8
  assert(config->select("subA", true) == 1);
  assert(config->put<const char *>("foo", "bar"));
  config->unwind(1);
  assert(config->put<const char *>("subB.bar", "baz"));
  config->unwind(1);
  assert(config->put<const char *>("7.subC.baz", "woof"));
  assert(config->push<const char *>("theEnd"));
  config->unwind(2);
  assert(config->putArray("woof.baz"));
  config->unwind();

  // Save, reload, and check JSON file
  assert(config->saveJSON("config.json"));
  config = ResourceManager::getActive()->get<Dictionary>("config.json");
  SDL_Log("config.json[thing]: %u\n", config->keep<Uint16>("thing", 1));
  assert(config->select("bar.woof.baz") == 3);
  SDL_Log("config.json[bar.woof.baz.0]: %s\n",
          config->keepAt<const char *>(0, "arf"));
  assert(config->selectIndex(7, true) == 1);
  SDL_Log("config.json[bar.woof.baz.7.subA.foo]: %s\n",
          config->keep<const char *>("subA.foo", "arf"));
  assert(config->select("subB", true) == 1);
  SDL_Log("config.json[bar.woof.baz.7.subB.bar]: %s\n",
          config->keep<const char *>("bar", "arf"));
  config->unwind(2);
  SDL_Log("config.json[bar.woof.baz.7.subC.baz]: %s\n",
          config->keep<const char *>("7.subC.baz", "arf"));
  config->unwind();
  SDL_Log("config.json[bar.woof.baz.5]: %s\n",
          config->keep<const char *>("bar.woof.baz.5", "arf"));

  // Enumeration tests
  assert(config->select("bar.woof") == 2);
  assert(config->begin() == UINT32_MAX && config->end() == UINT32_MAX);
  Uint32 objProps =
      config->enumerate(nullptr, [](Dictionary &dict, const String &key) {
        const char *val;
        if (dict.get<const char *>(nullptr, &val)) {
          SDL_Log("bar.woof['%s']: '%s'\n", key.c_str(), val);
        } else {
          SDL_Log("bar.woof['%s']: Not a string\n", key.c_str());
        }
        return true;
      });
  assert(objProps == 3);
  assert(config->select("baz") == 1);
  SDL_Log("bar.woof.baz -> %lu - %lu elements\n", config->end(),
          config->begin());
  Uint32 innerProps = 0;
  Uint32 outerProps = config->enumerateArray(
      nullptr, [&](Dictionary &dict, const Uint32 &index) {
        ++innerProps;
        const char *val;
        if (dict.get<const char *>(nullptr, &val)) {
          SDL_Log("bar.woof.baz[%lu]: '%s'\n", index, val);
        } else {
          dict.select("subB");
          if (dict.get<const char *>("bar", &val)) {
            SDL_Log("bar.woof.baz[%lu].subB.bar: '%s'\n", index, val);
          } else {
            SDL_Log("bar.woof.baz[%lu].subB.bar: **Not a string**\n", index);
          }
        }
        return true;
      });
  assert(innerProps == 6 && outerProps == 6);

  app.destroy();
  return 0;
}
