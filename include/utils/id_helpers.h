/****************************************************
 * id_helpers.h: Convert Strings <-> Ids            *
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

#include "types.h"

namespace renity {
template <typename T>
inline Id getId(T val) {
  // TODO: Reimplement in XXH3 if/when we switch to that
  return (Id)std::hash<String>{}(toString(val));
}

// Have to add specializations because toString(String) and
// std::hash<const char*> do not behave how one would logically expect...
template <>
inline Id getId(String val) {
  // TODO: Reimplement in XXH3 if/when we switch to that
  return (Id)std::hash<String>{}(val);
}

template <>
inline Id getId(const char* val) {
  return getId(String(val));
}

/** Maps SDL event type ids to generic strings */
inline const char* getSDLEventTypeName(const Uint32 type) {
  switch (type) {
    /* User-requested quit */
    case SDL_EVENT_QUIT:
      return "EVT_QUIT";
    /* The application is being terminated by the OS Called on iOS in
     * applicationWillTerminate() Called on Android in onDestroy() */
    case SDL_EVENT_TERMINATING:
      return "EVT_TERMINATING";
    /* The application is low on memory, free memory if possible. Called on iOS
     * in applicationDidReceiveMemoryWarning() Called on Android in
     * onLowMemory() */
    case SDL_EVENT_LOW_MEMORY:
      return "EVT_LOW_MEMORY";
    /* The application is about to enter the background Called on iOS in
     * applicationWillResignActive() Called on Android in onPause() */
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
      return "EVT_WILL_ENTER_BACKGROUND";
    /* The application did enter the background and may not get CPU for some
     * time Called on iOS in applicationDidEnterBackground() Called on Android
     * in onPause() */
    case SDL_EVENT_DID_ENTER_BACKGROUND:
      return "EVT_DID_ENTER_BACKGROUND";
    /* The application is about to enter the foreground Called on iOS in
     * applicationWillEnterForeground() Called on Android in onResume() */
    case SDL_EVENT_WILL_ENTER_FOREGROUND:
      return "EVT_WILL_ENTER_FOREGROUND";
    /* The application is now interactive Called on iOS in
     * applicationDidBecomeActive() Called on Android in onResume() */
    case SDL_EVENT_DID_ENTER_FOREGROUND:
      return "EVT_DID_ENTER_FOREGROUND";
    /* The user's locale preferences have changed. */
    case SDL_EVENT_LOCALE_CHANGED:
      return "EVT_LOCALE_CHANGED";
    /* The system theme changed */
    case SDL_EVENT_SYSTEM_THEME_CHANGED:
      return "EVT_SYSTEM_THEME_CHANGED";
    /* Display orientation has changed to data1 */
    case SDL_EVENT_DISPLAY_ORIENTATION:
      return "EVT_DISPLAY_ORIENTATION";
    /* Display has been added to the system */
    case SDL_EVENT_DISPLAY_CONNECTED:
      return "EVT_DISPLAY_CONNECTED";
    /* Display has been removed from the system */
    case SDL_EVENT_DISPLAY_DISCONNECTED:
      return "EVT_DISPLAY_DISCONNECTED";
    /* Display has changed position */
    case SDL_EVENT_DISPLAY_MOVED:
      return "EVT_DISPLAY_MOVED";
    /* Display has changed content scale */
    case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
      return "EVT_DISPLAY_CONTENT_SCALE_CHANGED";
    /* System specific event */
    case SDL_EVENT_SYSWM:
      return "EVT_SYSWM";
    /* Window has been shown */
    case SDL_EVENT_WINDOW_SHOWN:
      return "EVT_WINDOW_SHOWN";
    /* Window has been hidden */
    case SDL_EVENT_WINDOW_HIDDEN:
      return "EVT_WINDOW_HIDDEN";
    /* Window has been exposed and should be redrawn */
    case SDL_EVENT_WINDOW_EXPOSED:
      return "EVT_WINDOW_EXPOSED";
    /* Window has been moved to data1, data2 */
    case SDL_EVENT_WINDOW_MOVED:
      return "EVT_WINDOW_MOVED";
    /* Window has been resized to data1xdata2 */
    case SDL_EVENT_WINDOW_RESIZED:
      return "EVT_WINDOW_RESIZED";
    /* The pixel size of the window has changed to data1xdata2 */
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      return "EVT_WINDOW_PIXEL_SIZE_CHANGED";
    /* Window has been minimized */
    case SDL_EVENT_WINDOW_MINIMIZED:
      return "EVT_WINDOW_MINIMIZED";
    /* Window has been maximized */
    case SDL_EVENT_WINDOW_MAXIMIZED:
      return "EVT_WINDOW_MAXIMIZED";
    /* Window has been restored to normal size and position */
    case SDL_EVENT_WINDOW_RESTORED:
      return "EVT_WINDOW_RESTORED";
    /* Window has gained mouse focus */
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
      return "EVT_WINDOW_MOUSE_ENTER";
    /* Window has lost mouse focus */
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
      return "EVT_WINDOW_MOUSE_LEAVE";
    /* Window has gained keyboard focus */
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
      return "EVT_WINDOW_FOCUS_GAINED";
    /* Window has lost keyboard focus */
    case SDL_EVENT_WINDOW_FOCUS_LOST:
      return "EVT_WINDOW_FOCUS_LOST";
    /* The window manager requests that the window be closed */
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      return "EVT_WINDOW_CLOSE_REQUESTED";
    /* Window is being offered a focus (should SetWindowInputFocus() on itself
     * or a subwindow, or ignore) */
    case SDL_EVENT_WINDOW_TAKE_FOCUS:
      return "EVT_WINDOW_TAKE_FOCUS";
    /* Window had a hit test that wasn't SDL_HITTEST_NORMAL */
    case SDL_EVENT_WINDOW_HIT_TEST:
      return "EVT_WINDOW_HIT_TEST";
    /* The ICC profile of the window's display has changed */
    case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
      return "EVT_WINDOW_ICCPROF_CHANGED";
    /* Window has been moved to display data1 */
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
      return "EVT_WINDOW_DISPLAY_CHANGED";
    /* Window display scale has been changed */
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
      return "EVT_WINDOW_DISPLAY_SCALE_CHANGED";
    /* The window has been occluded */
    case SDL_EVENT_WINDOW_OCCLUDED:
      return "EVT_WINDOW_OCCLUDED";
    /* The window with the associated ID is being or has been destroyed. If this
     * message is being handled in an event watcher, the window handle is still
     * valid and can still be used to retrieve any userdata associated with the
     * window. Otherwise, the handle has already been destroyed and all
     * resources associated with it are invalid */
    case SDL_EVENT_WINDOW_DESTROYED:
      return "EVT_WINDOW_DESTROYED";
    /* Key pressed */
    case SDL_EVENT_KEY_DOWN:
      return "EVT_KEY_DOWN";
    /* Key released */
    case SDL_EVENT_KEY_UP:
      return "EVT_KEY_UP";
    /* Keyboard text editing (composition) */
    case SDL_EVENT_TEXT_EDITING:
      return "EVT_TEXT_EDITING";
    /* Keyboard text input */
    case SDL_EVENT_TEXT_INPUT:
      return "EVT_TEXT_INPUT";
    /* Keymap changed due to a system event such as an input language or
     * keyboard layout change. */
    case SDL_EVENT_KEYMAP_CHANGED:
      return "EVT_KEYMAP_CHANGED";
    /* Extended keyboard text editing (composition) */
    case SDL_EVENT_TEXT_EDITING_EXT:
      return "EVT_TEXT_EDITING_EXT";
    /* Mouse moved */
    case SDL_EVENT_MOUSE_MOTION:
      return "EVT_MOUSE_MOTION";
    /* Mouse button pressed */
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      return "EVT_MOUSE_BUTTON_DOWN";
    /* Mouse button released */
    case SDL_EVENT_MOUSE_BUTTON_UP:
      return "EVT_MOUSE_BUTTON_UP";
    /* Mouse wheel motion */
    case SDL_EVENT_MOUSE_WHEEL:
      return "EVT_MOUSE_WHEEL";
    /* Joystick axis motion */
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      return "EVT_JOYSTICK_AXIS_MOTION";
    /* Joystick hat position change */
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      return "EVT_JOYSTICK_HAT_MOTION";
    /* Joystick button pressed */
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      return "EVT_JOYSTICK_BUTTON_DOWN";
    /* Joystick button released */
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
      return "EVT_JOYSTICK_BUTTON_UP";
    /* A new joystick has been inserted into the system */
    case SDL_EVENT_JOYSTICK_ADDED:
      return "EVT_JOYSTICK_ADDED";
    /* An opened joystick has been removed */
    case SDL_EVENT_JOYSTICK_REMOVED:
      return "EVT_JOYSTICK_REMOVED";
    /* Joystick battery level change */
    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
      return "EVT_JOYSTICK_BATTERY_UPDATED";
    /* Joystick update is complete (disabled by default) */
    case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
      return "EVT_JOYSTICK_UPDATE_COMPLETE";
    /* Gamepad axis motion */
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
      return "EVT_GAMEPAD_AXIS_MOTION";
    /* Gamepad button pressed */
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      return "EVT_GAMEPAD_BUTTON_DOWN";
    /* Gamepad button released */
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      return "EVT_GAMEPAD_BUTTON_UP";
    /* A new gamepad has been inserted into the system */
    case SDL_EVENT_GAMEPAD_ADDED:
      return "EVT_GAMEPAD_ADDED";
    /* An opened gamepad has been removed */
    case SDL_EVENT_GAMEPAD_REMOVED:
      return "EVT_GAMEPAD_REMOVED";
    /* The gamepad mapping was updated */
    case SDL_EVENT_GAMEPAD_REMAPPED:
      return "EVT_GAMEPAD_REMAPPED";
    /* Gamepad touchpad was touched */
    case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
      return "EVT_GAMEPAD_TOUCHPAD_DOWN";
    /* Gamepad touchpad finger was moved */
    case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
      return "EVT_GAMEPAD_TOUCHPAD_MOTION";
    /* Gamepad touchpad finger was lifted */
    case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
      return "EVT_GAMEPAD_TOUCHPAD_UP";
    /* Gamepad sensor was updated */
    case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
      return "EVT_GAMEPAD_SENSOR_UPDATE";
    /* Gamepad update is complete (disabled by default) */
    case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
      return "EVT_GAMEPAD_UPDATE_COMPLETE";
    /*  */
    case SDL_EVENT_FINGER_DOWN:
      return "EVT_FINGER_DOWN";
    /*  */
    case SDL_EVENT_FINGER_UP:
      return "EVT_FINGER_UP";
    /*  */
    case SDL_EVENT_FINGER_MOTION:
      return "EVT_FINGER_MOTION";
    /* The clipboard or primary selection changed */
    case SDL_EVENT_CLIPBOARD_UPDATE:
      return "EVT_CLIPBOARD_UPDATE";
    /* The system requests a file open */
    case SDL_EVENT_DROP_FILE:
      return "EVT_DROP_FILE";
    /* text/plain drag-and-drop event */
    case SDL_EVENT_DROP_TEXT:
      return "EVT_DROP_TEXT";
    /* A new set of drops is beginning (NULL filename) */
    case SDL_EVENT_DROP_BEGIN:
      return "EVT_DROP_BEGIN";
    /* Current set of drops is now complete (NULL filename) */
    case SDL_EVENT_DROP_COMPLETE:
      return "EVT_DROP_COMPLETE";
    /* Position while moving over the window */
    case SDL_EVENT_DROP_POSITION:
      return "EVT_DROP_POSITION";
    /* A new audio device is available */
    case SDL_EVENT_AUDIO_DEVICE_ADDED:
      return "EVT_AUDIO_DEVICE_ADDED";
    /* An audio device has been removed. */
    case SDL_EVENT_AUDIO_DEVICE_REMOVED:
      return "EVT_AUDIO_DEVICE_REMOVED";
    /* A sensor was updated */
    case SDL_EVENT_SENSOR_UPDATE:
      return "EVT_SENSOR_UPDATE";
    /* The render targets have been reset and their contents need updating */
    case SDL_EVENT_RENDER_TARGETS_RESET:
      return "EVT_RENDER_TARGETS_RESET";
    /* The device has been reset and all textures need to be recreated */
    case SDL_EVENT_RENDER_DEVICE_RESET:
      return "EVT_RENDER_DEVICE_RESET";
    /* Signals the end of an event poll cycle */
    case SDL_EVENT_POLL_SENTINEL:
      return "EVT_POLL_SENTINEL";
    /* User Event */
    default:
      if (type >= SDL_EVENT_USER && type <= SDL_EVENT_LAST) return "EVT_USER";
      break;
  }

  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
              "getSDLEventTypeName: Unknown SDL event type %u.", type);
  return "UNKNOWN";
}

inline String getSDLEventTypeString(const Uint32 type) {
  return String(getSDLEventTypeName(type));
}

inline ActionId getSDLEventTypeActionId(const Uint32 type) {
  String name = getSDLEventTypeString(type);
  if (name == "UNKNOWN") return 0;
  return getId(name);
}
}  // namespace renity
