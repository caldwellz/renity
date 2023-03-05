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

#include "3rdparty/imgui/backends/imgui_impl_sdl3.h"
#include "3rdparty/imgui/backends/imgui_impl_sdlrenderer.h"
#include "3rdparty/imgui/imgui.h"
#include "config.h"
#include "types.h"

namespace renity {
struct Window::Impl {
  Impl() {
    window = nullptr;
    renderer = nullptr;
    guiCtx = nullptr;
    guiClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    title = "Renity Window";
    position = Point2Di32(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    size = Dimension2Di32(1, 1);
    fullscreen = false;
    fullscreenMode = NULL;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;
  ImGuiContext *guiCtx;
  ImVec4 guiClearColor;
  String title;
  Point2Di32 position;
  Dimension2Di32 size;
  bool fullscreen;
  SDL_DisplayMode *fullscreenMode;
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
static int eventProcessor(void *userdata, SDL_Event *event) {
  Window *w = (Window *)userdata;
  const bool windowEvent = event->type >= SDL_EVENT_WINDOW_FIRST &&
                           event->type <= SDL_EVENT_WINDOW_LAST;
  const bool currentWindowEvent =
      windowEvent && event->window.windowID == w->getWindowID();

  // ImGui wants all event types, but filter out ones for other windows and ones
  // we handle here to avoid confusing it.
  if (!windowEvent) {
    ImGui_ImplSDL3_ProcessEvent(event);
    return 1;
  }
  if (!currentWindowEvent) {
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                 "Window %i ignored event type %i for window %i.\n",
                 w->getWindowID(), event->window.type, event->window.windowID);
    return 1;
  }

  SDL_Renderer *renderer = w->pimpl_->renderer;
  switch (event->window.type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      w->close();
      break;
    case SDL_EVENT_WINDOW_RESIZED:
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                   "Window::eventProcessor: Window resizing to %ix%i screen "
                   "coordinates.\n",
                   event->window.data1, event->window.data2);
      SDL_SetRenderLogicalPresentation(
          renderer, event->window.data1, event->window.data2,
          SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_BEST);
      break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "Window::eventProcessor: Window resized to %ix%i actual pixels.\n",
          event->window.data1, event->window.data2);
      break;
    default:
      ImGui_ImplSDL3_ProcessEvent(event);
      SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
                   "Unhandled window event type %i on windowId %i.\n",
                   event->window.type, event->window.windowID);
  }
  return 0;
}

RENITY_API bool Window::isOpen() const { return (pimpl_->renderer); }

RENITY_API bool Window::open() {
  // Status checks
  if (isOpen()) return true;
  if (!IMGUI_CHECKVERSION()) {
    SDL_SetError("ImGui version mismatch: %s", IMGUI_VERSION);
    return false;
  }

  // Create and check the window
  uint32_t flags =
      SDL_WINDOW_RESIZABLE | (pimpl_->fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
  pimpl_->window = SDL_CreateWindow(pimpl_->title.c_str(), pimpl_->position.x(),
                                    pimpl_->position.y(), pimpl_->size.width(),
                                    pimpl_->size.height(), flags);
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

  // Create and check the renderer
  // TODO: Allow choosing a rendering backend
  pimpl_->renderer =
      SDL_CreateRenderer(pimpl_->window, NULL,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!pimpl_->renderer) {
    // Try again without VSync
    pimpl_->renderer =
        SDL_CreateRenderer(pimpl_->window, NULL, SDL_RENDERER_ACCELERATED);
    if (!pimpl_->renderer) {
      // Try again with a VSync'd software fallback
      pimpl_->renderer =
          SDL_CreateRenderer(pimpl_->window, NULL,
                             SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
      if (!pimpl_->renderer) {
        // Try again with non-VSync software fallback
        pimpl_->renderer =
            SDL_CreateRenderer(pimpl_->window, NULL, SDL_RENDERER_SOFTWARE);
        if (!pimpl_->renderer) {
          SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
                          "renity::Window::open: Could not create any kind of "
                          "renderer: '%s'",
                          SDL_GetError());
          SDL_DestroyWindow(pimpl_->window);
          pimpl_->window = nullptr;
          return false;
        }
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                     "renity::Window::open: Accelerated renderer not available "
                     "('%s'); using non-vsync software fallback.",
                     SDL_GetError());
      } else {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                     "renity::Window::open: Accelerated renderer not available "
                     "('%s'); using vsync software fallback.",
                     SDL_GetError());
      }
    } else {
      SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO,
                  "renity::Window::open: Accelerated vsync not available "
                  "('%s'); using non-vsync accelerated renderer.",
                  SDL_GetError());
    }
  }

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
      "Window displaying %ix%i screen coordinates using %ix%i actual pixels.\n",
      pimpl_->size.width(), pimpl_->size.height(), trueW, trueH);
#endif

  // Set up ImGui
  pimpl_->guiCtx = ImGui::CreateContext();
  ImGui::SetCurrentContext(pimpl_->guiCtx);
  ImGuiIO &io = ImGui::GetIO();
  io.MouseDrawCursor = true;
#ifdef RENITY_DEBUG
  // Don't hide the OS cursor
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#endif
  // TODO: See ImGui_ImplSDL3_Init for how to set custom cursors (ImGui loads
  // SDL's there by default) Enable Keyboard Controls io.ConfigFlags |=
  // ImGuiConfigFlags_NavEnableKeyboard; Enable Gamepad Controls io.ConfigFlags
  // |= ImGuiConfigFlags_NavEnableGamepad;
  ImGui::StyleColorsDark();  // TODO: Use custom application theme
  if (!pimpl_->guiCtx ||
      !ImGui_ImplSDL3_InitForSDLRenderer(pimpl_->window, pimpl_->renderer) ||
      !ImGui_ImplSDLRenderer_Init(pimpl_->renderer)) {
    SDL_SetError("ImGui failed to initialize");
    return false;
  }

  // Watch for window events
  SDL_AddEventWatch(eventProcessor, this);

  // Clear the initial backbuffer
  SDL_SetRenderDrawColor(pimpl_->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pimpl_->renderer);

  // Start an initial ImGui frame
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  return activate();
}

RENITY_API void Window::close() {
  if (pimpl_->guiCtx) {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(pimpl_->guiCtx);
    pimpl_->guiCtx = nullptr;
  }

  if (pimpl_->renderer) {
    SDL_DestroyRenderer(pimpl_->renderer);
    pimpl_->renderer = nullptr;
  }

  if (pimpl_->window) {
    SDL_DestroyWindow(pimpl_->window);
    SDL_DelEventWatch(eventProcessor, this);
    pimpl_->window = nullptr;
  }
}

RENITY_API bool Window::activate() {
  if (!pimpl_->window) return false;

  // Bring the window to the front and focus the input
  ImGui::SetCurrentContext(pimpl_->guiCtx);
  SDL_RaiseWindow(pimpl_->window);

  // Full input grab may not always be desirable because the mouse gets
  // confined to the window. So, only grab the input if it's already
  // grabbed somewhere, and not by the current window.
  SDL_Window *grabbed = SDL_GetGrabbedWindow();
  if (grabbed && grabbed != pimpl_->window)
    SDL_SetWindowGrab(pimpl_->window, SDL_TRUE);

  return true;
}

RENITY_API bool Window::update() {
  if (pimpl_->renderer == nullptr) return false;

  // Render last frame's GUI
  ImGui::Render();
  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

  // Swap buffers and prepare the new one
  const int renderStatus =
      SDL_RenderPresent(pimpl_->renderer) |
      SDL_SetRenderDrawColor(pimpl_->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) |
      SDL_RenderClear(pimpl_->renderer);
  if (renderStatus != 0) return false;

  // Start a new ImGui frame
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  return true;
}

RENITY_API SDL_WindowID Window::getWindowID() const {
  return SDL_GetWindowID(pimpl_->window);
}

RENITY_API SDL_Renderer *Window::getRenderer() const {
  return pimpl_->renderer;
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
  if (pimpl_->renderer) {
    int w, h;
    SDL_GetCurrentRenderOutputSize(pimpl_->renderer, &w, &h);
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
