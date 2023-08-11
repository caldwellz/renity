/****************************************************
 * InputMapper.cc: Convert user input to Actions    *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "InputMapper.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>

#include "Action.h"
#include "ActionHandler.h"
#include "ActionManager.h"
#include "Dictionary.h"
#include "HashTable.h"
#include "ResourceManager.h"
#include "resources/default_input_maps.h"
#include "utils/id_helpers.h"
#include "utils/physfsrwops.h"

namespace renity {
// Input source, OR'd with the 2nd byte of the btnOrKey field
enum InputSource : Uint8 {
  // Binary literals because C++14 FTW
  INPUT_KEYBOARD = 0b00000010,
  // Hashed "special" keys (volume, play/pause, etc.) automatically set low bit
  INPUT_KEYBOARD_SPECIAL = 0b00000011,
  INPUT_MOUSE_BTN = 0b00001000,
  INPUT_MOUSE_AXIS = 0b00001001,
  // Touch matches mouse mask
  INPUT_TOUCH_BTN = 0b00001100,
  INPUT_TOUCH_AXIS = 0b00001101,
  INPUT_JOYSTICK_BTN = 0b00100000,
  INPUT_JOYSTICK_AXIS = 0b00100001,
  // Gamepad matches joystick mask
  INPUT_GAMEPAD_BTN = 0b00110000,
  INPUT_GAMEPAD_AXIS = 0b00110001,
  // High 2 bits are device number, meaning up to 4 joysticks/gamepads for now
  INPUT_INSTANCE1 = 0b00000000,
  INPUT_INSTANCE2 = 0b01000000,
  INPUT_INSTANCE3 = 0b10000000,
  INPUT_INSTANCE4 = 0b11000000,
};

// Bounds for SDL input event types, since SDL doesn't provide them itself
enum SDLInputEventGroups : Uint16 {
  EVT_ANY_INPUT_FIRST = 0x300,
  EVT_KEYBOARD_FIRST = 0x300,
  EVT_KEYBOARD_MAX = 0x3FF,
  EVT_MOUSE_FIRST = 0x400,
  EVT_MOUSE_MAX = 0x4FF,
  EVT_JOYSTICK_FIRST = 0x600,
  EVT_JOYSTICK_MAX = 0x64F,
  EVT_GAMEPAD_FIRST = 0x650,
  EVT_GAMEPAD_MAX = 0x6FF,
  EVT_TOUCH_FIRST = 0x700,
  EVT_TOUCH_MAX = 0x74F,
  // Current end is actually 0x703, but we'll give touch events room to grow
  EVT_ANY_INPUT_LAST = 0x70A,
  EVT_INPUT_GROUP_MASK = 0xF00
};

struct InputMapper::Impl : public ActionHandler {
  Impl() {}
  ~Impl() {}

  // Handles input mapping requests.
  // TODO: Make thread-safe with a reader/writer lock
  void handleAction(const ActionCategoryId categoryId, const Action *action) {
    // Input types & hashes are distributed via Unmapped*Input actions; handlers
    // elsewhere should attach actId's and send them back via InputMappingChange
    static const ActionId mapChange =
        ActionManager::getActive()->assignCategory("InputMappingChange",
                                                   "InputChange");
    // static const ActionId devChange = ActionManager::getActive()->
    // assignCategory("InputDeviceChange", "InputChange");

    if (action->getId() == mapChange) {
      ActionId actId = action->getDataAs<ActionId>(0);
      Uint32 inputHash = action->getDataAs<Uint32>(1);
      mapDict.putIndex<ActionId>(inputHash, actId);
#ifdef RENITY_DEBUG
      // Only auto-save in debug since it removes the abiity to cancel changes
      // TODO: Add action triggers for load/save?
      mapDict.save("keybinds.json");
#endif
    }
  }

  ActionId getButtonAction(Uint8 source, Uint16 btn, Uint8 clickCount) {
    // Check for multiclicks in descending order
    ActionId actId;
    for (Uint8 clicks = clickCount; clicks >= 1; --clicks) {
      // Check only keyboard-type modifiers until joystick/gp support is added
      if (mapDict.getIndex<ActionId>(
              getButtonHash(source, btn, clicks, (Uint16)SDL_GetModState()),
              &actId))
        return actId;

      // Games also use modifiers as regular action keys, so check unmodded too
      if (mapDict.getIndex<ActionId>(getButtonHash(source, btn, clicks, 0),
                                     &actId))
        return actId;
    }

    // Must be unmapped
    return 0;
  }

  Uint32 getButtonHash(Uint16 source, Uint16 btnOrKey, Uint8 clicks,
                       Uint16 mods) {
    // Merge input source with button/key
    Uint32 type = source << 8 | btnOrKey;

    // Ignore "locking" modifiers (Num/Caps/etc.), capping mods at 12 bits
    const Uint32 nonLockingMods =
        mods & (SDL_KMOD_SHIFT | SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_GUI);

    // Mask click count at 4 bits (i.e. modulo 16) to fit it below mods
    const Uint32 maskedClicks = clicks & 0x0F;

    const Uint32 out = type << 16 | nonLockingMods << 4 | maskedClicks;
    SDL_LogVerbose(
        SDL_LOG_CATEGORY_INPUT,
        "InputMapper::getButtonHash: (0x%04x, 0x%04x, 0x%04x, 0x%02x) "
        "-> 0x%08x",
        source, btnOrKey, clicks, mods, out);
    return out;
  }

  ActionId getAxisAction(Uint8 source, Uint32 instance, Uint16 axis) {
    ActionId actId;
    if (mapDict.getIndex<ActionId>(getAxisHash(source, instance, axis),
                                   &actId)) {
      return actId;
    }
    return 0;
  }

  Uint32 getAxisHash(Uint8 source, Uint32 instance, Uint16 axis) {
    // TODO: Check real SDL instancing and map it to InputSource devices
    const Uint32 deviceNumber = instance & 0;

    // Merge device number with input source
    const Uint32 type = deviceNumber << 6 | source;

    // TODO: Find a use for 2nd byte
    const Uint32 out = type << 24 | axis;
    SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT,
                   "InputMapper::getAxisHash: (0x%02x, 0x%08x, 0x%04x) "
                   "-> 0x%08x",
                   source, instance, axis, out);
    return out;
  }

  int handleKeyboardEvent(const SDL_Event *event) {
    static const ActionId textInput =
        ActionManager::getActive()->assignCategory("TextInput", "Input");
    static const ActionId unmappedInput =
        ActionManager::getActive()->assignCategory("UnmappedButtonInput",
                                                   "Input");
    const char *evtTypeName = getSDLEventTypeName(event->type);
    String inputText;

    // Handle text input / text editing events
    switch (event->type) {
      // Don't duplicate input; text input (TEXT_* events) should be the only
      // keyboard thing forwarded when there's an active text input focus
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
        if (SDL_TextInputActive()) {
          return 0;
        }
        break;
      case SDL_EVENT_TEXT_INPUT:
        // TODO: Check actual editing events to see if start is ever < 0
        inputText = event->text.text;
        ActionManager::getActive()->post(Action(
            textInput, {inputText, (Sint32)-1, (Sint32)inputText.length()}));
        return 0;
      case SDL_EVENT_TEXT_EDITING:
        inputText = event->edit.text;
        ActionManager::getActive()->post(Action(
            textInput, {inputText, event->edit.start, event->edit.length}));
        SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT,
                       "InputMapper::handleKeyboardEvent: %s '%s' %i:%i",
                       evtTypeName, event->edit.text, event->edit.start,
                       event->edit.length);
        return 0;
      case SDL_EVENT_TEXT_EDITING_EXT:
        inputText = event->editExt.text;
        ActionManager::getActive()->post(
            Action(textInput,
                   {inputText, event->editExt.start, event->editExt.length}));
        SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT,
                       "InputMapper::handleKeyboardEvent: %s '%s' %i:%i",
                       evtTypeName, event->editExt.text, event->editExt.start,
                       event->editExt.length);
        return 0;
        // case SDL_EVENT_KEYMAP_CHANGED:
      default:
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                     "InputMapper::handleKeyboardEvent: Unhandled event %s",
                     evtTypeName);
        return 0;
    }

    // Handle KEY_DOWN / KEY_UP events, but ignore repeats outside of text input
    const SDL_Keysym sym = event->key.keysym;
    const bool isPrintable = sym.sym >= 0x20 && sym.sym <= 0x7E;
    if (event->key.repeat) {
      SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT,
                     "InputMapper::handleKeyboardEvent: Ignoring repeat %i of "
                     "non-text-input '%c' (%s, 0x%04x).",
                     event->key.repeat, isPrintable ? sym.sym : ' ',
                     SDL_GetKeyName(sym.sym), sym.scancode);
      return 0;
    }

    const bool pressed = event->key.state == SDL_PRESSED;
    ActionId actId = getButtonAction(INPUT_KEYBOARD, (Uint16)sym.scancode, 1);
    if (!actId) {
      // TODO: Add a function/feature to translate input hash to GUI combo text
      Uint32 inputHash = getButtonHash(INPUT_KEYBOARD, (Uint16)sym.scancode, 1,
                                       (Uint16)SDL_GetModState());
      SDL_LogVerbose(
          SDL_LOG_CATEGORY_INPUT,
          "InputMapper::handleKeyboardEvent: No mapping for '%c' (%s, "
          "0x%04x) with 0x%04x mods (0x%08x hash).",
          isPrintable ? sym.sym : ' ', SDL_GetKeyName(sym.sym), sym.scancode,
          sym.mod, inputHash);
      ActionManager::getActive()->post(
          Action(unmappedInput, {inputHash, pressed}));
      return 0;
    }
    ActionManager::getActive()->post(Action(actId, {pressed}));
    return 0;
  }

  int handleMouseEvent(const SDL_Event *event) {
    static const ActionId unmappedInput =
        ActionManager::getActive()->assignCategory("UnmappedAxisInput",
                                                   "Input");

    // Handle button input
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
      const SDL_MouseButtonEvent btn = event->button;
      const bool pressed = btn.state == SDL_PRESSED;
      ActionId actId = getButtonAction(INPUT_MOUSE_BTN, btn.button, btn.clicks);
      if (!actId) {
        // TODO: Add a function/feature to translate inputId to GUI combo text
        SDL_LogVerbose(
            SDL_LOG_CATEGORY_INPUT,
            "InputMapper::handleMouseEvent: No mapping for button 0x%02x.",
            btn.button);
        ActionManager::getActive()->post(
            Action(unmappedInput,
                   {getButtonHash(INPUT_MOUSE_BTN, btn.button, btn.clicks,
                                  (Uint16)SDL_GetModState()),
                    pressed}));
        return 0;
      }
      ActionManager::getActive()->post(Action(actId, {pressed}));
      return 0;
    }

    // Handle axis input (motion or wheel)
    Uint32 instance;
    Uint16 axis;
    float xrel, yrel;
    switch (event->type) {
      case SDL_EVENT_MOUSE_MOTION:
        instance = event->motion.which;
        axis = 0;
        xrel = event->motion.xrel;
        yrel = event->motion.yrel;
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        instance = event->wheel.which;
        axis = 1;
        // Will almost always be whole pixels. Keep as floats to mimic other
        // axis inputs, but normalize direction if needed.
        xrel = event->wheel.x;
        yrel = event->wheel.y;
        if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
          xrel *= -1.0f;
          yrel *= -1.0f;
        }
        break;
      default:
        SDL_LogDebug(
            SDL_LOG_CATEGORY_INPUT,
            "InputMapper::handleMouseEvent: Unhandled event type 0x%08x",
            event->type);
        return 0;
    }

    ActionId actId = getAxisAction(INPUT_MOUSE_AXIS, instance, axis);
    if (!actId) {
      // TODO: Add a function/feature to translate inputId to GUI combo text
      SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT,
                     "InputMapper::handleMouseEvent: Unmapped axis 0x%04x on "
                     "instance 0x%08x",
                     axis, instance);
      ActionManager::getActive()->post(
          Action(unmappedInput,
                 {getAxisHash(INPUT_MOUSE_AXIS, instance, axis), false}));
      return 0;
    }
    ActionManager::getActive()->post(Action(actId, {xrel, yrel}));
    return 0;
  }

  String mapPath;
  Dictionary mapDict;
};

int inputEventProcessor(void *userdata, SDL_Event *event) {
  // Skip it if it's not an input event
  if (event->type < EVT_ANY_INPUT_FIRST || event->type > EVT_ANY_INPUT_LAST) {
    return 1;
  }
  InputMapper::Impl *pimpl_ = (InputMapper::Impl *)userdata;

  // TODO: Handle device additions/removals and bind/unbind as appropriate
  // static const Id inputDeviceCat = getId("InputDeviceChange");

  // Extract just the input group bits and distribute the event appropriately
  const Uint32 evtGroup = event->type & EVT_INPUT_GROUP_MASK;
  SDL_LogVerbose(
      SDL_LOG_CATEGORY_INPUT,
      "InputMapper::inputEventProcessor: Processing input event type "
      "%s (0x%04x) in group 0x%04x.",
      getSDLEventTypeName(event->type), event->type, evtGroup);
  switch (evtGroup) {
    case EVT_KEYBOARD_FIRST:
      return pimpl_->handleKeyboardEvent(event);
    case EVT_MOUSE_FIRST:
      return pimpl_->handleMouseEvent(event);
    // Touch will probably share a lot of functionality with mouse
    // case EVT_TOUCH_FIRST:
    // Joystick and Gamepad are numerically in the same group
    // case EVT_JOYSTICK_FIRST:
    default:
      SDL_LogDebug(
          SDL_LOG_CATEGORY_INPUT,
          "InputMapper::inputEventProcessor: Unhandled input event type "
          "%s (0x%04x).",
          getSDLEventTypeName(event->type), event->type);
  }
  return 0;
}

RENITY_API InputMapper::InputMapper(const char *loadPath) {
  pimpl_ = new Impl();
  SDL_AddEventWatch(inputEventProcessor, pimpl_);
  pimplHolder_ = ActionHandlerPtr(pimpl_);

  ActionManager::getActive()->assignCategory("InputMappingChange",
                                             "InputChange");
  ActionManager::getActive()->subscribe(pimplHolder_, "InputChange");
  load(loadPath);
}

RENITY_API InputMapper::~InputMapper() {
  SDL_DelEventWatch(inputEventProcessor, pimpl_);
  // pimplHolder_ will handle pimpl deletion
}

RENITY_API void InputMapper::load(const char *path) {
  SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
               "InputMapper::load: Loading mapping from '%s'",
               path ? path : "<nullptr>");

  // Load built-in default maps if no path supplied
  SDL_RWops *ops =
      path ? PHYSFSRWOPS_openRead(path)
           : SDL_RWFromConstMem(
                 "{}", 3);  // pDefaultInputMapData, pDefaultInputMapSize);
  pimpl_->mapDict.load(ops);
}

RENITY_API bool InputMapper::save(const char *path) {
  SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
               "InputMapper::save: Saving mapping to '%s'",
               path ? path : "<nullptr>");
  return pimpl_->mapDict.save(path);
}
}  // namespace renity
