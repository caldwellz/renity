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

#include "3rdparty/imgui/imgui.h"
#include "Sprite.h"
#include "config.h"
#include "types.h"
#include "version.h"

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

  // Show app info in debug mode
#ifdef RENITY_DEBUG
  const char *headlessMode = headless ? "headless" : "non-headless";
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
               "Application::initialize: Initializing %s application on %s.\n",
               headlessMode, SDL_GetPlatform());

  PHYSFS_Version compiled;
  PHYSFS_Version linked;
  PHYSFS_VERSION(&compiled);
  PHYSFS_getLinkedVersion(&linked);
  SDL_LogDebug(
      SDL_LOG_CATEGORY_SYSTEM,
      "PhysFS versions: %d.%d.%d (compiled against) vs %d.%d.%d (linked).\n",
      compiled.major, compiled.minor, compiled.patch, linked.major,
      linked.minor, linked.patch);
  PHYSFS_init(pimpl_->executableName);
#endif

  // Set up PhysFS
  if (!PHYSFS_isInit()) {
    SDL_SetError("Could not init PhysFS: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  const char *baseDir = PHYSFS_getBaseDir();
  const char *prefDir = PHYSFS_getPrefDir(PUBLISHER_NAME, PRODUCT_NAME);
  if (!PHYSFS_mount(baseDir, "/", 0)) {
    SDL_SetError("Could not mount PhysFS baseDir '%s': %s", baseDir,
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  if (!PHYSFS_mount(prefDir, "/", 1) || !PHYSFS_setWriteDir(prefDir)) {
    SDL_SetError(
        "Could not mount PhysFS prefDir '%s' using publisher '%s', product "
        "'%s': %s",
        prefDir, PUBLISHER_NAME, PRODUCT_NAME,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  // PHYSFS_setRoot(PHYSFS_getBaseDir(), "/assets");

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
      bounds.w *= 0.75;
      bounds.h *= 0.75;
      pimpl_->window.useFullscreen(false, true);
    }
    pimpl_->window.size(renity::Dimension2Di(bounds.w, bounds.h));
    if (!pimpl_->window.open()) {
      return false;
    }
    pimpl_->renderer = pimpl_->window.getRenderer();
    // SDL_SetRenderVSync(pimpl_->renderer, 0);
    Sprite spriteA("epic2.png");
    spriteA.draw();
    Sprite spriteB("epic2.png");
    spriteB.draw();
    SDL_Log("*** Initial Sprites going out of scope ***\n");
  }
  return true;
}

RENITY_API int Application::run() {
  SDL_Event event;
  bool keepGoing = true;
  bool show_demo_window, show_another_window;
  Uint32 frames = 0;
  Uint64 lastFrameTime = SDL_GetTicksNS();
  Uint64 fpsTime = 0;
  float fps = 1.0f;
  Sprite spriteA("epic2.png");
  Sprite spriteB("epic2.png");
  spriteA.setPosition({100, 100});
  spriteB.setPosition({100, 100});
  srand(SDL_GetTicksNS());
  spriteA.setMoveHeading(rand());
  spriteB.setMoveHeading(rand());
  while (keepGoing) {
    const Uint64 timeDelta = SDL_GetTicksNS() - lastFrameTime;
    lastFrameTime += timeDelta;
    fpsTime += timeDelta;
    if (fpsTime >= SDL_NS_PER_SECOND) {
      fps = (float)frames / ((float)fpsTime / SDL_NS_PER_SECOND);
      frames = 0;
      fpsTime = 0;
    }
    ++frames;

    const double moveSpeed = 600.0f * ((double)timeDelta / SDL_NS_PER_SECOND);
    spriteA.setMoveSpeed(moveSpeed);
    spriteB.setMoveSpeed(moveSpeed);
    int x = spriteA.getPosition().x();
    int y = spriteA.getPosition().y();
    if (x < 0 || x > pimpl_->window.size().width()) spriteA.bounceHorizontal();
    if (y < 0 || y > pimpl_->window.size().height()) spriteA.bounceVertical();
    x = spriteB.getPosition().x();
    y = spriteB.getPosition().y();
    if (x < 0 || x > pimpl_->window.size().width()) spriteB.bounceHorizontal();
    if (y < 0 || y > pimpl_->window.size().height()) spriteB.bounceVertical();
    spriteA.move();
    spriteB.move();
    spriteA.draw();
    spriteB.draw();

    // ImGUI demo
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                      // and append into it.

      ImGui::Text("This is some useful text.");
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window);  // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

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

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin(
          "Another Window",
          &show_another_window);  // Pass a pointer to our bool variable (the
                                  // window will have a closing button that will
                                  // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me")) show_another_window = false;
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
