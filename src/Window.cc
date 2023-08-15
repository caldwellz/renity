/****************************************************
 * Window.cc: Window management class               *
 * Copyright (C) 2021-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Window.h"

#include <SDL3/SDL.h>

#include "3rdparty/imgui/backends/imgui_impl_opengl3.h"
#include "3rdparty/imgui/backends/imgui_impl_sdl3.h"
#include "3rdparty/imgui/imgui.h"
#include "Action.h"
#include "ActionManager.h"
#include "ResourceManager.h"
#include "config.h"
#include "gl3.h"
#include "types.h"
#include "utils/id_helpers.h"
#include "version.h"

namespace renity {
Window *currentWindow = nullptr;
struct Window::Impl {
  Impl() {
    window = nullptr;
    glContext = nullptr;
    guiCtx = nullptr;
    clearColor = {0, 0, 0, 255};
    guiClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    title = PRODUCT_NAME;
    position = Point2Di32(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    size = Dimension2Di32(1, 1);
    fullscreen = false;
    fullscreenMode = nullptr;
    vsyncState = 0;
    wantToClose = false;
  }

  SDL_Window *window;
  SDL_GLContext glContext;
  ImGuiContext *guiCtx;
  ResourceManager resMgr;
  SDL_Color clearColor;
  ImVec4 guiClearColor;
  String title;
  Point2Di32 position;
  Dimension2Di32 size;
  bool fullscreen;
  SDL_DisplayMode *fullscreenMode;
  int vsyncState;
  bool wantToClose;
};

RENITY_API Window::Window() {
  // Initialize with default settings
  pimpl_ = new Impl();
}

RENITY_API Window::~Window() {
  this->close();
  delete this->pimpl_;
}

// TODO: Make this thread-safe, probably via a mutex in close()
int windowEventProcessor(void *userdata, SDL_Event *event) {
  Window *w = static_cast<Window *>(userdata);
  const bool windowEvent = event->type >= SDL_EVENT_WINDOW_FIRST &&
                           event->type <= SDL_EVENT_WINDOW_LAST;
  const bool currentWindowEvent =
      windowEvent && event->window.windowID == w->getWindowID();

  // ImGui wants all event types, but filter out ones for other windows and ones
  // we handle here to avoid confusing it.
  if (event->type == SDL_EVENT_QUIT) {
    w->pimpl_->wantToClose = true;
    return 1;
  }
  if (!windowEvent) {
    if (event->type != SDL_EVENT_POLL_SENTINEL)
      SDL_LogVerbose(
          SDL_LOG_CATEGORY_VIDEO,
          "Window::windowEventProcessor: Sending event %s (0x%04x) to GUI",
          getSDLEventTypeName(event->type), event->type);
    ImGui_ImplSDL3_ProcessEvent(event);
    return 1;
  }
  if (!currentWindowEvent) {
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                 "Window %i ignored event type %i for window %i.\n",
                 w->getWindowID(), event->window.type, event->window.windowID);
    return 1;
  }

  switch (event->window.type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    case SDL_EVENT_WINDOW_DESTROYED:
      w->pimpl_->wantToClose = true;
      return 0;
    case SDL_EVENT_WINDOW_RESIZED:
      SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "Window::windowEventProcessor: Window resizing to %ix%i screen "
          "coordinates.\n",
          event->window.data1, event->window.data2);
      /* TODO: Replace logical presentation / DPI logic
      SDL_SetRenderLogicalPresentation(
          renderer, event->window.data1, event->window.data2,
          SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_BEST);
      */
      break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "Window::windowEventProcessor: Window resized to %ix%i "
                   "actual pixels.\n",
                   event->window.data1, event->window.data2);
      // TODO: Move this to a window event handler in the renderer
      glViewport(0, 0, event->window.data1, event->window.data2);
      ImGui_ImplSDL3_ProcessEvent(event);
      break;
    // A bunch of event types that we know we don't currently care about
    case SDL_EVENT_WINDOW_SHOWN:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_OCCLUDED:
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_MOVED:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_MAXIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
    // case SDL_EVENT_WINDOW_FOCUS_GAINED:
    // case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_TAKE_FOCUS:
      break;
    default:
      ImGui_ImplSDL3_ProcessEvent(event);
      SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                   "Window::windowEventProcessor: Unhandled window event type "
                   "%i on windowId %i.",
                   event->window.type, event->window.windowID);
  }

  // Forward current-window events on to the action queue
  ActionManager::getActive()->post(Action(getSDLEventTypeActionId(event->type),
                                          {w->getWindowID(), userdata}));

  return 0;
}

RENITY_API Window *Window::getActive() { return currentWindow; }

RENITY_API bool Window::isOpen() const {
  return pimpl_->glContext && !pimpl_->wantToClose;
}

RENITY_API bool Window::open() {
  // Status checks
  if (isOpen()) return true;
  if (!IMGUI_CHECKVERSION()) {
    SDL_SetError("ImGui version mismatch: %s", IMGUI_VERSION);
    return false;
  }

  // Set common GL attributes
  // TODO: Try hardware acceleration first, then log and try without - default
  // is to allow either one SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
  // SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

  // Attempt to use an OpenGL ES 3.0 profile first, with no deprecated functions
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  int ctxFlags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifdef RENITY_DEBUG
  ctxFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, ctxFlags);

  // Enable native IME for ImGui
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  // Create and check the window
  // TODO: Try again with desktop OpenGL core profile if needed
  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                   (pimpl_->fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
  pimpl_->window = SDL_CreateWindowWithPosition(
      pimpl_->title.c_str(), pimpl_->position.x(), pimpl_->position.y(),
      pimpl_->size.width(), pimpl_->size.height(), flags);
  if (!pimpl_->window) return false;

  // Set a fullscreen mode if requested, bailing out on error
  if (pimpl_->fullscreen &&
      (SDL_SetWindowFullscreenMode(pimpl_->window, pimpl_->fullscreenMode) !=
       0)) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_VIDEO,
        "renity::Window::open: Could not set requested fullscreen mode: '%s'",
        SDL_GetError());
    SDL_DestroyWindow(pimpl_->window);
    pimpl_->window = nullptr;
    return false;
  }
  SDL_ShowWindow(pimpl_->window);

  // TODO: Split out all this logic if/when a Vulkan backend is added
  // Create the rendering context
  pimpl_->glContext = SDL_GL_CreateContext(pimpl_->window);
  if (!pimpl_->glContext) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_VIDEO,
        "renity::Window::open: Could not create OpenGL context: '%s'",
        SDL_GetError());
    SDL_DestroyWindow(pimpl_->window);
    pimpl_->window = nullptr;
    return false;
  }

  // Activate context and load the functions
  // This has to be done before activate() since we don't have an ImGui ctx yet
  SDL_GL_MakeCurrent(pimpl_->window, pimpl_->glContext);
  if (flextInit() != 0) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_VIDEO,
        "renity::Window::open: Could not load OpenGL functions: '%s'",
        SDL_GetError());
    close();
    return false;
  }

  // Log finalized GL info
  int major, minor;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
  SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
               "OpenGL info:\n  GL_VENDOR: '%s'\n  GL_RENDERER: '%s'\n  "
               "GL_VERSION: %s\n  Context version: %i.%i\n  GLSL version: %s",
               glGetString(GL_VENDOR), glGetString(GL_RENDERER),
               glGetString(GL_VERSION), major, minor,
               glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Default to using vsync, but here we don't care if it actually succeeds
  vsync(true);
  /*
    // Handle HighDPI by automatically scaling renderer output. As of writing,
    // SDL3 auto-adjusts the actual window size, but not the presentation.
    SDL_SetRenderLogicalPresentation(
        pimpl_->renderer, pimpl_->size.width(), pimpl_->size.height(),
        SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_BEST);

    // Log the initial window size
    #ifdef RENITY_DEBUG
      int trueW, trueH;
      SDL_GetRenderOutputSize(pimpl_->renderer, &trueW, &trueH);
      SDL_LogDebug(
          SDL_LOG_CATEGORY_VIDEO,
          "Window displaying %ix%i screen coordinates using %ix%i actual
    pixels.\n", pimpl_->size.width(), pimpl_->size.height(), trueW, trueH);
    #endif
    */
  // Set up ImGui
  pimpl_->guiCtx = ImGui::CreateContext();
  ImGui::SetCurrentContext(pimpl_->guiCtx);
  ImGuiIO &io = ImGui::GetIO();
  io.MouseDrawCursor = true;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  // io.ConfigFlags |=
  //     ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
#ifdef RENITY_DEBUG
  // Don't hide the OS cursor
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#endif
  // TODO: See ImGui_ImplSDL3_Init for how to set custom cursors (ImGui loads
  // SDL's there by default)
  ImGui::StyleColorsDark();  // TODO: Use custom application theme
  if (!pimpl_->guiCtx ||
      !ImGui_ImplSDL3_InitForOpenGL(pimpl_->window, pimpl_->glContext) ||
      !ImGui_ImplOpenGL3_Init()) {
    SDL_SetError("ImGui failed to initialize");
    return false;
  }

  // Register window events under the "Window" action category
  // Doesn't work right in the constructor, so we'll do it here
  for (Uint32 type = SDL_EVENT_WINDOW_FIRST; type <= SDL_EVENT_WINDOW_LAST;
       ++type) {
    ActionManager::getActive()->assignCategory(getSDLEventTypeString(type),
                                               "Window");
  }

  // TEXT_INPUT events seem to be enabled by default on at least Windows;
  // pause them until we actually want "text input" (instead of plain keys).
  // Not done in InputMapper because it only takes effect after a window opens
  SDL_StopTextInput();

  // Watch for window events
  SDL_AddEventWatch(windowEventProcessor, this);

  // Activate the current clear color, then clear the initial buffers
  clearColor(pimpl_->clearColor);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Start an initial ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  pimpl_->wantToClose = false;
  return activate();
}

RENITY_API void Window::close() {
  SDL_LogVerbose(SDL_LOG_CATEGORY_VIDEO, "Window::close: Closing window %i",
                 getWindowID());

  if (pimpl_->guiCtx) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(pimpl_->guiCtx);
    pimpl_->guiCtx = nullptr;
  }

  if (pimpl_->glContext) {
    SDL_GL_DeleteContext(pimpl_->glContext);
    pimpl_->glContext = nullptr;
  }

  if (pimpl_->window) {
    SDL_DelEventWatch(windowEventProcessor, this);
    SDL_DestroyWindow(pimpl_->window);
    pimpl_->window = nullptr;
  }

  if (currentWindow == this) currentWindow = nullptr;
}

RENITY_API bool Window::activate() {
  if (!isOpen()) return false;

  // Activate OpenGL context and reload context-specific functions
  SDL_GL_MakeCurrent(pimpl_->window, pimpl_->glContext);
  if (flextInit() != 0) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_VIDEO,
        "renity::Window::activate: Could not (re)load OpenGL functions: '%s'",
        SDL_GetError());
    pimpl_->wantToClose = true;
    return false;
  }

  // Bring the window to the front and focus the input
  ImGui::SetCurrentContext(pimpl_->guiCtx);
  SDL_RaiseWindow(pimpl_->window);

  // Full input grab may not always be desirable because the mouse gets
  // confined to the window. So, only grab the input if it's already
  // grabbed somewhere, and not by the current window.
  SDL_Window *grabbed = SDL_GetGrabbedWindow();
  if (grabbed && grabbed != pimpl_->window)
    SDL_SetWindowGrab(pimpl_->window, SDL_TRUE);

  // GL resources are generally bound to the context they were created under.
  // As such, each Window needs its own ResourceManager context.
  pimpl_->resMgr.activate();

  currentWindow = this;
  return true;
}

RENITY_API bool Window::update() {
  if (!isOpen() || currentWindow != this) {
    return false;
  }

  // Render last frame's GUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // NOTE: This will be required on macOS if we start using framebuffer objects
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Swap buffers and prepare the new one
  if (SDL_GL_SwapWindow(pimpl_->window) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                 "renity::Window::update: Buffer swap failed: '%s'",
                 SDL_GetError());
    return false;
  }
  // Clear the backbuffer if we're not overwriting it on every frame
  // glClear(GL_COLOR_BUFFER_BIT);

  // Reload any shaders, meshes, etc. that have changed on disk
  pimpl_->resMgr.update();

  // Start a new ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  return true;
}

RENITY_API SDL_Color Window::clearColor() const { return pimpl_->clearColor; }

RENITY_API void Window::clearColor(const SDL_Color color) {
  pimpl_->clearColor = color;
  if (pimpl_->glContext) {
    // SDL colors are 0-255 while GL expects 0.0-1.0
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                 color.a / 255.0f);
  }
}

RENITY_API bool Window::vsync() const { return (pimpl_->vsyncState != 0); }

RENITY_API bool Window::vsync(bool enable) {
  if (SDL_GL_GetCurrentContext() != pimpl_->glContext) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO,
                "renity::Window::vsync: Ignoring vsync change request on "
                "inactive context.");
    return false;
  }

  if (enable) {
    if (SDL_GL_SetSwapInterval(-1) == 0) {
      SDL_LogDebug(
          SDL_LOG_CATEGORY_VIDEO,
          "renity::Window::vsync: Successfully enabled adaptive vsync.");
      pimpl_->vsyncState = -1;
      return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                 "renity::Window::vsync: Could not enable adaptive vsync "
                 "('%s'); trying regular.",
                 SDL_GetError());
    if (SDL_GL_SetSwapInterval(1) == 0) {
      pimpl_->vsyncState = 1;
      return true;
    }
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                 "renity::Window::vsync: Could not enable vsync: '%s'",
                 SDL_GetError());
    pimpl_->vsyncState = 0;
    return false;
  }

  if (SDL_GL_SetSwapInterval(0) == 0) {
    pimpl_->vsyncState = 0;
    return true;
  }
  SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
               "renity::Window::vsync: Could not disable vsync: '%s'",
               SDL_GetError());
  // vsyncState is not modified
  return false;
}

RENITY_API SDL_WindowID Window::getWindowID() const {
  return SDL_GetWindowID(pimpl_->window);
}

RENITY_API SDL_GLContext Window::getGlContext() const {
  return pimpl_->glContext;
}

RENITY_API String Window::title() const {
  if (pimpl_->window) pimpl_->title = SDL_GetWindowTitle(pimpl_->window);

  return pimpl_->title;
}

RENITY_API void Window::title(const String new_title) {
  pimpl_->title = new_title;
  if (pimpl_->window) SDL_SetWindowTitle(pimpl_->window, new_title.c_str());
}

RENITY_API Point2Di32 Window::position() const {
  if (pimpl_->window) {
    int x, y;
    SDL_GetWindowPosition(pimpl_->window, &x, &y);
    pimpl_->position.x(x);
    pimpl_->position.y(y);
  }
  return pimpl_->position;
}

RENITY_API void Window::position(const Point2Di32 &new_pos) {
  pimpl_->position = new_pos;
  if (pimpl_->window)
    SDL_SetWindowPosition(pimpl_->window, new_pos.x(), new_pos.y());
}

RENITY_API void Window::centerPosition() {
  pimpl_->position.x(SDL_WINDOWPOS_CENTERED);
  pimpl_->position.y(SDL_WINDOWPOS_CENTERED);
  if (pimpl_->window)
    SDL_SetWindowPosition(pimpl_->window, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
}

RENITY_API Dimension2Di32 Window::size() const {
  if (pimpl_->window) {
    int w, h;
    SDL_GetWindowSize(pimpl_->window, &w, &h);
    pimpl_->size.width(w);
    pimpl_->size.height(h);
  }
  return pimpl_->size;
}

RENITY_API Dimension2Di32 Window::sizeInPixels() const {
  if (pimpl_->window) {
    int w, h;
    SDL_GetWindowSizeInPixels(pimpl_->window, &w, &h);
    return Dimension2Di32(w, h);
  }
  return size();
}

RENITY_API bool Window::size(const Dimension2Di32 &new_size) {
  if (new_size.width() > 0) pimpl_->size.width(new_size.width());

  if (new_size.height() > 0) pimpl_->size.height(new_size.height());

  if (pimpl_->window) {
    SDL_SetWindowSize(pimpl_->window, pimpl_->size.width(),
                      pimpl_->size.height());
    if (pimpl_->fullscreenMode) return useFullscreen(true, false);
  }

  return true;
}

RENITY_API bool Window::isFullscreen() const {
  return (pimpl_->fullscreenMode != 0);
}

RENITY_API bool Window::useFullscreen(bool fullscreen,
                                      bool use_native_resolution) {
  if (fullscreen) {
    // TODO: Convert requested window size to an SDL_DisplayMode here, or better
    // yet, automatically use a borderless screen-size window
    pimpl_->fullscreen = true;
    pimpl_->fullscreenMode = nullptr;
    if (!use_native_resolution)
      SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                   "renity::Window::useFullscreen: Non-native fullscreen mode "
                   "support is not yet implemented.\n");
  } else {
    pimpl_->fullscreen = false;
    pimpl_->fullscreenMode = nullptr;
  }

  if (pimpl_->window) {
    if (SDL_SetWindowFullscreen(pimpl_->window,
                                fullscreen ? SDL_TRUE : SDL_FALSE) != 0)
      return false;
    if (SDL_SetWindowFullscreenMode(pimpl_->window, pimpl_->fullscreenMode) !=
        0)
      return false;
  }

  return true;
}
}  // namespace renity
