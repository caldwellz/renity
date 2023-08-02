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

#ifdef RENITY_DEBUG
#include "3rdparty/dmon/dmon.h"
#endif
#include "3rdparty/imgui/imgui.h"
#include "ActionHandler.h"
#include "ActionManager.h"
#include "ResourceManager.h"
#include "Window.h"
// #include "Sprite.h"
#include "config.h"
#include "types.h"
#include "utils/id_helpers.h"
#include "version.h"

namespace renity {
struct Application::Impl {
  Impl(const char *argv0) {
    context = nullptr;
    executableName = argv0;
    headless = false;
  }

  Window window;
  ActionManager actionMgr;
  ResourceManager resMgr;
  SDL_GLContext context;
  const char *executableName;
  bool headless;
};

#ifdef RENITY_DEBUG
class ActionLogger : public ActionHandler {
  void handleAction(const ActionCategoryId categoryId, const Action *action) {
    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "ActionLogger::handleAction: categoryId:0x%08x, "
        "actionId:0x%08x, createdAt: %.1f secs, dataA:0x%08x, dataB:0x%08x",
        categoryId, action->getId(), action->getCreatedAt() / 1000.0f,
        action->getDataA(), action->getDataB());
  }
};
#endif

RENITY_API Application::Application(int argc, char *argv[]) {
  // TODO: Command-line flags parsing
  pimpl_ = new Impl(argv ? argv[0] : nullptr);

#ifdef RENITY_DEBUG
  // Redirect logs to a file and turn on debug logs
  // freopen("stderr.txt", "w", stderr);
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  // Watch for file changes
  dmon_init();

  // Register an Action logger for various categories
  ActionHandlerPtr actLogger(new ActionLogger);
  pimpl_->actionMgr.subscribe(actLogger, getId("Window"));
#else
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#endif
}

RENITY_API Application::~Application() {
#ifdef RENITY_DEBUG
  dmon_deinit();
#endif
  this->destroy();
  delete this->pimpl_;
}

RENITY_API bool Application::initialize(bool headless) {
  pimpl_->headless = headless;

  // Show app info and watch for resource changes in debug mode
#ifdef RENITY_DEBUG
  const char *headlessMode = headless ? "headless" : "non-headless";
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Application::initialize: Initializing %s debug-mode "
               "application on %s.\n",
               headlessMode, SDL_GetPlatform());

  PHYSFS_Version compiled;
  PHYSFS_Version linked;
  PHYSFS_VERSION(&compiled);
  PHYSFS_getLinkedVersion(&linked);
  SDL_LogVerbose(
      SDL_LOG_CATEGORY_SYSTEM,
      "PhysFS versions: %d.%d.%d (compiled against) vs %d.%d.%d (linked).\n",
      compiled.major, compiled.minor, compiled.patch, linked.major,
      linked.minor, linked.patch);
#endif

  // Set up PhysFS
  PHYSFS_init(pimpl_->executableName);
  if (!PHYSFS_isInit()) {
    SDL_SetError("Could not init PhysFS: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  const char *baseDir = PHYSFS_getBaseDir();
  const char *prefDir = PHYSFS_getPrefDir(PUBLISHER_NAME, PRODUCT_NAME);
  if (!PHYSFS_mount(prefDir, "/", 0) || !PHYSFS_setWriteDir(prefDir)) {
    SDL_SetError(
        "Could not mount PhysFS prefDir '%s' using publisher '%s', product "
        "'%s': %s",
        prefDir, PUBLISHER_NAME, PRODUCT_NAME,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  if (!PHYSFS_mount(baseDir, "/", 1)) {
    SDL_SetError("Could not mount PhysFS baseDir '%s': %s", baseDir,
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  // PHYSFS_setRoot(PHYSFS_getBaseDir(), "/assets");
  pimpl_->resMgr.activate();

  // Log final search paths in debug mode
#ifdef RENITY_DEBUG
  char **pathList = PHYSFS_getSearchPath();
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "-- PhysFS search paths:\n");
  for (char **pathIter = pathList; *pathIter != NULL; ++pathIter) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s\n", *pathIter);
  }
  PHYSFS_freeList(pathList);
#endif

  // Initialize SDL
  Uint32 systems = SDL_INIT_TIMER | SDL_INIT_EVENTS;
  if (!headless) {
    systems |= SDL_INIT_VIDEO | SDL_INIT_AUDIO;
  }
  if (SDL_Init(systems) != 0) return false;

  // TODO: Load window/display settings from app config
  if (!headless) {
    // Default to a window taking up 3/4 of the screen, if over a certain size
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    SDL_Rect bounds = {0, 0, 0, 0};
    SDL_GetDisplayBounds(display, &bounds);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                 "Detected bounds for primary display (%i): %ix%i\n", display,
                 bounds.w, bounds.h);
    if (bounds.w <= 1366 || bounds.h <= 768) {
      pimpl_->window.useFullscreen(true, true);
    } else {
      // Use 75% of the screen, but without float conversions
      bounds.w = (bounds.w / 4) * 3;
      bounds.h = (bounds.h / 4) * 3;
      pimpl_->window.useFullscreen(false, true);
    }
    pimpl_->window.size(renity::Dimension2Di(bounds.w, bounds.h));
    if (!pimpl_->window.open()) {
      return false;
    }
    pimpl_->context = pimpl_->window.getGlContext();
  }

  return true;
}

RENITY_API int Application::run() {
  SDL_Event event;
  bool keepGoing = true;
  bool show_demo_window = true;
  Uint32 frames = 0;
  Uint64 lastFrameTime = SDL_GetTicksNS();
  Uint64 fpsTime = 0;
  float fps = 1.0f;
  // Vector<Sprite> sprites;
  Uint64 spriteCount = 0;
  srand((Uint32)SDL_GetTicksNS());
  getWindow()->vsync(false);
  getWindow()->clearColor({0, 0, 200, 255});

  while (keepGoing) {
    // Recalculate displayed FPS every second
    const Uint64 timeDelta = SDL_GetTicksNS() - lastFrameTime;
    lastFrameTime += timeDelta;
    fpsTime += timeDelta;
    if (fpsTime >= SDL_NS_PER_SECOND) {
      fps = (float)frames / ((float)fpsTime / SDL_NS_PER_SECOND);
      frames = 0;
      fpsTime = 0;
    }
    ++frames;

    // Add more sprites until we start dropping frames
    // const double realtimeFPS = (double)SDL_NS_PER_SECOND / timeDelta;
    /*
    if (spriteCount < 10) {  // realtimeFPS > 59.9999) {
      sprites.emplace_back("epic.png");
      sprites.back().setPosition({pimpl_->window.size().width() / 2,
                                  pimpl_->window.size().height() / 2});
      sprites.back().setMoveHeading(rand());
      ++spriteCount;
    }

    // Move & draw sprites
    const double moveSpeed = 650.0f * ((double)timeDelta / SDL_NS_PER_SECOND);
    for (auto &s : sprites) {
      s.setMoveSpeed(moveSpeed);
      int x = s.getPosition().x();
      int y = s.getPosition().y();
      if (x < 0 || x > pimpl_->window.size().width()) s.bounceHorizontal();
      if (y < 0 || y > pimpl_->window.size().height()) s.bounceVertical();
      s.move();
      s.draw();
    }
    */
    // ImGUI demo
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                      // and append into it.

      ImGui::Text("Rendering %llu sprites.", spriteCount);
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window);  // Edit bools storing our window open/close state

      ImGui::SliderFloat(
          "float", &f, 0.0f,
          1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
      // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3
      // floats representing a color

      if (ImGui::Button(
              "Button"))  // Buttons return true when clicked (most widgets
                          // return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / fps,
                  fps);
      ImGui::End();
    }

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
