/***************************************************
* Texture.cc: Texture management class             *
* Copyright (C) 2021 by Zach Caldwell              *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "Texture.h"

#include <SDL2/SDL_render.h>

#include "config.h"
#include "types.h"
#include "utils/texture_utils.h"
#include "Window.h"

namespace renity {
struct Texture::Impl {
    /** Defaults to full-screen VSynced mode at the native screen resolution. */
    Impl() {
        renderer = nullptr;
        tex = nullptr;
        imgPath = "";
        colorKeyPosition.x = colorKeyPosition.y = -1;
        colorKeyEnabled = 0;
    }

    SDL_Renderer* renderer;
    SDL_Texture* tex;
    String imgPath;
    SDL_Point colorKeyPosition;
    int colorKeyEnabled;
};

RENITY_API Texture::Texture() {
    // Initialize with default settings
    pimpl_ = new Impl();
}

 
RENITY_API Texture::Texture(const Window& window) {
    pimpl_ = new Impl();
    pimpl_->renderer = window.getRenderer();
}


RENITY_API Texture::Texture(const Window& window, const String& path) {
    pimpl_ = new Impl();
    pimpl_->renderer = window.getRenderer();
    load(path);
}


RENITY_API Texture::~Texture() {
    unload();
    delete pimpl_;
}


RENITY_API bool Texture::load(const String path) {
    unload(); // * This is why path is pass-by-value rather than by-reference *
    pimpl_->tex = RENITY_LoadPhysTextureEx(pimpl_->renderer, path.c_str()
                , pimpl_->colorKeyEnabled, &pimpl_->colorKeyPosition);

    if (pimpl_->tex) {
        pimpl_->imgPath = path;
        return true;
    }

    return false;
}


RENITY_API void Texture::unload() {
    if (pimpl_->tex) {
        SDL_DestroyTexture(pimpl_->tex);
        pimpl_->tex = nullptr;
    }
    pimpl_->imgPath = "";
}


RENITY_API bool Texture::enableColorKey(const Point2Di& keyPosition) {
    pimpl_->colorKeyEnabled = 1;
    pimpl_->colorKeyPosition = keyPosition.toSDLPoint();
    if (isValid())
        return load(pimpl_->imgPath);

    return true;
}


RENITY_API void Texture::disableColorKey() {
    pimpl_->colorKeyEnabled = 0;
    pimpl_->colorKeyPosition.x = pimpl_->colorKeyPosition.y = -1;
    if (isValid())
        load(pimpl_->imgPath);
}


RENITY_API bool Texture::isColorKeyEnabled() {
    return (pimpl_->colorKeyEnabled > 0);
}


RENITY_API bool Texture::isValid() const {
    return (pimpl_->tex && pimpl_->renderer);
}


RENITY_API String Texture::getImagePath() const {
    return pimpl_->imgPath;
}


RENITY_API Dimension2Di Texture::getSize() const {
    if (isValid()) {
        int w, h;
        if (0 == SDL_QueryTexture(pimpl_->tex, NULL, NULL, &w, &h))
            return Dimension2Di(w, h);
    }

    return Dimension2Di(0, 0);
}


RENITY_API bool Texture::setWindow(const Window& window) {
    pimpl_->renderer = window.getRenderer();
    if (pimpl_->tex)
        return load(pimpl_->imgPath);

    return (pimpl_->renderer != nullptr);
}


RENITY_API bool Texture::draw(const Rect2Di* source, const Rect2Di* dest
                            , const double& angle , const Point2Di* origin
                            , const bool& flipHorizontal, const bool& flipVertical) {
    if (isValid()) {
        SDL_Rect srcRect, destRect, *srcRectPtr = NULL, *destRectPtr = NULL;
        SDL_Point center, *centerPtr = NULL;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (source) {
            srcRect = source->toSDLRect();
            srcRectPtr = &srcRect;
        }
        if (dest) {
            destRect = dest->toSDLRect();
            destRectPtr = &destRect;
        }
        if (origin) {
            center = origin->toSDLPoint();
            centerPtr = &center;
        }
        if (flipHorizontal) {
            flip = (SDL_RendererFlip) (flip | SDL_FLIP_HORIZONTAL);
        if (flipVertical)
            flip = (SDL_RendererFlip) (flip | SDL_FLIP_VERTICAL);
        }

        return (0 == SDL_RenderCopyEx(pimpl_->renderer, pimpl_->tex, srcRectPtr
                                    , destRectPtr, angle, centerPtr, flip));
    }

    return false;
}
}
