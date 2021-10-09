/***************************************************
* Window.cc: Window management class               *
* Copyright (C) 2021 by Zach Caldwell              *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "Window.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include "config.h"
#include "types.h"

namespace renity {
struct Window::Impl {
    /** Defaults to full-screen VSynced mode at the native screen resolution. */
    Impl() {
        window = nullptr;
        renderer = nullptr;
        title = "Renity Window";
        position = Point2Di32(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        size = Dimension2Di32(1, 1);
        fullscreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_Window* window;
    SDL_Renderer* renderer;
    String title;
    Point2Di32 position;
    Dimension2Di32 size;
    uint32_t fullscreenMode;
};


RENITY_API Window::Window() {
    // Initialize with default settings
    pimpl_ = new Impl();
}


RENITY_API Window::~Window() {
    this->close();
    delete this->pimpl_;
}


RENITY_API bool Window::isOpen() const {
    return (pimpl_->renderer);
}


RENITY_API bool Window::open() {
    // Status check
    if (isOpen())
        return true;

    // Create and check the window
    uint32_t flags = pimpl_->fullscreenMode | SDL_WINDOW_SHOWN
                   | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    pimpl_->window = SDL_CreateWindow(pimpl_->title.c_str(), pimpl_->position.x(),
                                      pimpl_->position.y(), pimpl_->size.width(),
                                      pimpl_->size.height(), flags);
    if (!pimpl_->window)
        return false;

    // Create and check the renderer
    flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    pimpl_->renderer = SDL_CreateRenderer(pimpl_->window, -1, flags);
    if (!pimpl_->renderer) {
        // Try again with no flags
        pimpl_->renderer = SDL_CreateRenderer(pimpl_->window, -1, 0);
        if (!pimpl_->renderer) {
            SDL_DestroyWindow(pimpl_->window);
            pimpl_->window = nullptr;
            return false;
        }
    }

    // Clear the initial backbuffer
    SDL_SetRenderDrawColor(pimpl_->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pimpl_->renderer);

    return activate();
}


RENITY_API void Window::close() {
    if (pimpl_->renderer) {
        SDL_DestroyRenderer(pimpl_->renderer);
        pimpl_->renderer = nullptr;
    }

    if (pimpl_->window) {
        SDL_DestroyWindow(pimpl_->window);
        pimpl_->window = nullptr;
    }
}


RENITY_API bool Window::activate() {
    if (!pimpl_->window)
        return false;

    // Bring the window to the front and focus the input
    SDL_RaiseWindow(pimpl_->window);

    // Full input grab may not always be desirable because the mouse gets
    // confined to the window. So, only grab the input if it's already
    // grabbed somewhere, and not by the current window.
    SDL_Window* grabbed = SDL_GetGrabbedWindow();
    if (grabbed && grabbed != pimpl_->window)
        SDL_SetWindowGrab(pimpl_->window, SDL_TRUE);

    return true;
}


RENITY_API bool Window::update() {
    if (pimpl_->renderer == nullptr)
        return false;

    // TODO: Handle SDL window events

    // Swap buffers and prepare the new one
    SDL_RenderPresent(pimpl_->renderer);
    SDL_SetRenderDrawColor(pimpl_->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    return (SDL_RenderClear(pimpl_->renderer) == 0);
}


RENITY_API SDL_Renderer* Window::getRenderer() const {
    return pimpl_->renderer;
}


RENITY_API String Window::title() const {
    if (pimpl_->window)
        pimpl_->title = SDL_GetWindowTitle(pimpl_->window);

    return pimpl_->title;
}


RENITY_API void Window::title(const String new_title) {
    pimpl_->title = new_title;
    if (pimpl_->window)
        SDL_SetWindowTitle(pimpl_->window, new_title.c_str());
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
        SDL_SetWindowPosition(pimpl_->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
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
        SDL_GetRendererOutputSize(pimpl_->renderer, &w, &h);
        return Dimension2Di32(w, h);
    }
    return size();
}


RENITY_API bool Window::size(const Dimension2Di32 &new_size) {
    if (new_size.width() > 0)
        pimpl_->size.width(new_size.width());

    if (new_size.height() > 0)
        pimpl_->size.height(new_size.height());

    if (pimpl_->window) {
        SDL_SetWindowSize(pimpl_->window, pimpl_->size.width(), pimpl_->size.height());
        if (pimpl_->fullscreenMode)
            return useFullscreen(true, false);
    }

    return true;
}


RENITY_API bool Window::isFullscreen() const {
    return (pimpl_->fullscreenMode != 0);
}


RENITY_API bool Window::useFullscreen(bool fullscreen, bool use_native_resolution) {
    if (fullscreen) {
        if (use_native_resolution)
            pimpl_->fullscreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            pimpl_->fullscreenMode = SDL_WINDOW_FULLSCREEN;
    } else {
        pimpl_->fullscreenMode = 0;
    }

    if (pimpl_->window) {
        if (SDL_SetWindowFullscreen(pimpl_->window, pimpl_->fullscreenMode) != 0)
            return false;
        if (pimpl_->fullscreenMode) {
            // Handle display mode (size) changes, since we might be called by size().
            if (SDL_SetWindowDisplayMode(pimpl_->window, nullptr) != 0)
                return false;
        }
    }

    return true;
}
}
