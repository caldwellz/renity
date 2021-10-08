/***************************************************
* Test - Image surface/texture utility functions   *
* Copyright (C) 2020-2021 by Zach Caldwell         *
****************************************************
* This Source Code Form is subject to the terms of *
* the Mozilla Public License, v. 2.0. If a copy of *
* the MPL was not distributed with this file, You  *
* can obtain one at http://mozilla.org/MPL/2.0/.   *
***************************************************/

#include "utils/texture_utils.h"

#include <assert.h>

#include <physfs.h>
#include <SDL2/SDL.h>
#include "resources/hood_png_zip.h"

int main(int argc, char* argv[])
{
    // Initialization and basic sanity checks
    PHYSFS_init(argv[0]);
    assert(PHYSFS_isInit());
    assert(PHYSFS_mountMemory(hoodArchive, hoodArchiveLen, NULL, hoodArchiveName, NULL, 1));
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window* window = SDL_CreateWindow("Image test", 0, 128, 128, 128, SDL_WINDOW_SHOWN);
    assert(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    assert(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Pure blue
    SDL_Point position = {0, 0};

    // Check LoadPhysTexture and LoadPhysTextureEx
    SDL_Texture* texture = RENITY_LoadPhysTexture(renderer, hoodFile);
    assert(texture);
    SDL_DestroyTexture(texture);
    texture = RENITY_LoadPhysTextureEx(renderer, hoodFile, SDL_TRUE, &position);
    assert(texture);

    // Display test
    SDL_Rect destRect = {32, 32, 64, 64};
    for (Uint8 i = 0; i < 2; ++i) {
        SDL_RenderClear(renderer);
        assert(SDL_RenderCopy(renderer, texture, NULL, &destRect) == 0);
        SDL_RenderPresent(renderer);
        SDL_Delay(750);
    }

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    PHYSFS_deinit();

    return 0;
}
