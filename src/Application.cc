/****************************************************
 * Application.cc: Game application management      *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Application.h"

#include <SDL3/SDL.h>
#include <physfs.h>

#include "config.h"
#include "types.h"

namespace renity {
struct Application::Impl {
  Impl(const char *argv0) {
    renderer = nullptr;
    executableName = argv0;
    headless = false;
  }

  Window window;
  SDL_Renderer *renderer;
  const char *executableName;
  bool headless;
};

RENITY_API Application::Application(int argc, char *argv[]) {
  // TODO: Command-line flags parsing
  pimpl_ = new Impl(argv[0]);

#ifdef RENITY_DEBUG
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#else
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#endif
}

RENITY_API Application::~Application() {
  this->destroy();
  delete this->pimpl_;
}

RENITY_API bool Application::initialize(bool headless) {
  pimpl_->headless = headless;
  const char *headlessMode = headless ? "headless" : "non-headless";
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Application::initialize: Initializing %s application on %s.\n",
               headlessMode, SDL_GetPlatform());

  PHYSFS_init(pimpl_->executableName);
  if (!PHYSFS_isInit()) {
    SDL_SetError(PHYSFS_getLastError());
    return false;
  }

  // TODO: Do application-specific mounts based on publisher and program strings
  // from version.h
  if (!PHYSFS_mount(".", "/", 1)) {
    SDL_SetError(PHYSFS_getLastError());
    return false;
  }

  Uint32 systems = SDL_INIT_TIMER | SDL_INIT_EVENTS;
  if (!headless) {
    systems |= SDL_INIT_VIDEO | SDL_INIT_AUDIO;
  }
  if (SDL_Init(systems) != 0) return false;

  // TODO: Load window settings from app config
  if (!headless) {
    pimpl_->window.useFullscreen(false, true);
    pimpl_->window.size(renity::Dimension2Di(800, 600));
    if (!pimpl_->window.open()) {
      return false;
    }
    pimpl_->renderer = pimpl_->window.getRenderer();
  }

  return true;
}

RENITY_API int Application::run() {
  SDL_Event event;
  bool keepGoing = true;
  while (keepGoing) {
    // Pump events, then clear them all out after subsystems react to the
    // updates, only listening for quit here. Subsystems should use
    // SDL_AddEventWatch(), SDL_FilterEvents(), or even SDL_PeepEvents() to get
    // the ones they're interested in.
    SDL_PumpEvents();
    if (!pimpl_->headless) {
      keepGoing = pimpl_->window.update();
    }
    if (!keepGoing) {
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "Application::run: Exit triggered by window.update().\n");
    }
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                     "Application::run: Exit triggered by SDL_EVENT_QUIT.\n");
        keepGoing = false;
      }
    }
  }

  return 0;
}

RENITY_API void Application::destroy() {
  pimpl_->window.close();
  SDL_Quit();
  PHYSFS_deinit();
}

RENITY_API Window *Application::getWindow() const { return &(pimpl_->window); }
}  // namespace renity
