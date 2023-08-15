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
#include "InputMapper.h"
#include "ResourceManager.h"
#include "Window.h"
#include "resources/GL_Mesh.h"
#include "resources/GL_ShaderProgram.h"
// #include "Sprite.h"

#include "config.h"
// #include "gl3.h"
#include "types.h"
#include "utils/id_helpers.h"
#include "utils/string_helpers.h"
#include "version.h"

namespace renity {
struct Application::Impl {
  explicit Impl(const char *argv0) {
    context = nullptr;
    executableName = argv0;
    headless = false;
  }

  Window window;
  ActionManager actionMgr;
  InputMapper inputMapper;
  SDL_GLContext context;
  const char *executableName;
  bool headless;
};

#ifdef RENITY_DEBUG
// Boilerplate to make visit() work
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

class ActionLogger : public ActionHandler {
  void logAny(size_t index, const PrimitiveVariant &any) {
    String str = std::visit(
        overloaded{[](auto arg) {
                     String s(typeName(arg));
                     return s + " " + toString(arg);
                   },
                   [](void *arg) {
                     char str[25];
                     snprintf(str, 25, "void* 0x%llx", (uintptr_t)arg);
                     return String(str);
                   },
                   [](const String &arg) { return String("String ") + arg; }},
        any);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "    %i: %s", index,
                 str.c_str());
  }

  void handleAction(const ActionCategoryId categoryId, const Action *action) {
    static ActionId discardedInput =
        ActionManager::getActive()->assignCategory("DebugIgnore", "Input");
    static ActionId changeInput = getId("InputMappingChange");
    static ActionId unmappedButton = getId("UnmappedButtonInput");
    static ActionId unmappedAxis = getId("UnmappedAxisInput");
    if (action->getId() == unmappedButton || action->getId() == unmappedAxis) {
      ActionManager::getActive()->post(
          Action(changeInput, {discardedInput, action->getData(0)}));
      SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "ActionLogger::handleAction: REGISTERING INPUT categoryId:0x%08x, "
          "actionId:0x%08x",
          categoryId, action->getId(), action->getCreatedAt() / 1000.0f);
    } else {
      SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "ActionLogger::handleAction: action: %s (0x%08x), category: "
          "%s (0x%08x), createdAt: %.1f secs, data:",
          action->getName().c_str(), action->getId(),
          ActionManager::getActive()->getNameFromId(categoryId).c_str(),
          categoryId, action->getCreatedAt() / 1000.0f);
      for (auto i = 0; i < action->getDataCount(); ++i) {
        logAny(i, action->getData(i));
      }
    }
  }
};
#endif

RENITY_API Application::Application(int argc, char *argv[]) {
#ifdef RENITY_DEBUG
  // Redirect logs to a file and turn on debug logs
  // freopen("stderr.txt", "w", stderr);
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#else
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#endif

  // TODO: Command-line flags parsing
  pimpl_ = new Impl(argv && argc ? argv[0] : nullptr);

#ifdef RENITY_DEBUG
  // Watch for file changes
  dmon_init();

  // Register an Action logger for various categories
  ActionHandlerPtr actLogger(new ActionLogger);
  pimpl_->actionMgr.subscribe(actLogger, "Window");
  pimpl_->actionMgr.subscribe(actLogger, "Input");
  pimpl_->actionMgr.subscribe(actLogger, "InputChange");
#endif
}

RENITY_API Application::~Application() {
#ifdef RENITY_DEBUG
  dmon_deinit();
#endif
  pimpl_->window.close();
  PHYSFS_deinit();
  delete this->pimpl_;
  SDL_Quit();
}

static PHYSFS_EnumerateCallbackResult mountAssetPaks(void *data,
                                                     const char *origdir,
                                                     const char *fname) {
  if (endsWith(fname, ".pkg")) {
    // Can't use origdir because it's in PhysFS notation, not platform-specific
    String filePath(PHYSFS_getBaseDir());
    filePath += fname;
    if (!PHYSFS_mount(filePath.c_str(), "/assets", 1)) {
      SDL_SetError("Could not mount asset pkg '%s': %s", filePath.c_str(),
                   PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
      return PHYSFS_ENUM_ERROR;
    }
  }
  return PHYSFS_ENUM_OK;
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

  // Set up PhysFS - needs to be done before Window resMgr activation
  PHYSFS_init(pimpl_->executableName);
  if (!PHYSFS_isInit()) {
    SDL_SetError("Could not init PhysFS: %s",
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  const char *baseDir = PHYSFS_getBaseDir();
  const char *prefDir = PHYSFS_getPrefDir(PUBLISHER_NAME, PRODUCT_NAME);

  // Mount loose files in the user's pref dir and then any mod packages
  if (!PHYSFS_mount(prefDir, "/profile", 0) || !PHYSFS_setWriteDir(prefDir)) {
    if (!PHYSFS_setWriteDir(baseDir)) {
      SDL_SetError(
          "Could not mount prefDir '%s' using publisher '%s' / product "
          "'%s': %s",
          prefDir, PUBLISHER_NAME, PRODUCT_NAME,
          PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
      return false;
    }
  }
  // TODO: Implement a mod ordering/loading system instead of direct overrides
  // PHYSFS_enumerate("/profile/Mods/", mountAssetPaks, (void *)prefDir);

  // Mount loose files in the application dir and then any asset packages
  if (!PHYSFS_mount(baseDir, "/", 1)) {
    SDL_SetError("Could not mount baseDir '%s': %s", baseDir,
                 PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  PHYSFS_enumerate("/", mountAssetPaks, nullptr);

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
    // Load user profile data
#ifdef RENITY_DEBUG
    pimpl_->inputMapper.load("keybinds.json");
#else
    pimpl_->inputMapper.load("keybinds.dat");
#endif

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
  bool show_demo_window = false;
  Uint32 frames = 0;
  Uint64 lastFrameTime = SDL_GetTicksNS();
  Uint64 fpsTime = 0;
  float fps = 1.0f, red = 0.0f, green = 0.4f, blue = 0.7f;
  // Vector<Sprite> sprites;
  Uint64 spriteCount = 0;
  srand((Uint32)SDL_GetTicksNS());
  // getWindow()->vsync(false);
  getWindow()->clearColor({0, 0, 200, 255});
  GL_Mesh::enableWireframe(false);
  GL_ShaderProgramPtr shader =
      ResourceManager::getActive()->get<GL_ShaderProgram>(
          "/assets/shaders/uniform.shader");
  GL_MeshPtr mesh =
      ResourceManager::getActive()->get<GL_Mesh>("/assets/meshes/pyramid.mesh");

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
      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                      // and append into it.

      // ImGui::Text("Rendering %llu sprites.", spriteCount);
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window);  // Edit bools storing our window open/close state

      ImGui::SliderFloat("Red", &red, 0.0f, 1.0f, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp);
      ImGui::SliderFloat("Green", &green, 0.0f, 1.0f, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp);
      ImGui::SliderFloat("Blue", &blue, 0.0f, 1.0f, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp);
      // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3
      // floats representing a color

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / fps,
                  fps);
      ImGui::End();
    }

    // Draw sample shape
    shader->setUniformBlock("ColorBlock", {red, green, blue, 0.5f});
    shader->use();
    mesh->use();
    mesh->draw();

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

RENITY_API Window *Application::getWindow() const { return &(pimpl_->window); }
}  // namespace renity
