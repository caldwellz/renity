/****************************************************
 * Test - Stateful texture-based sprite class       *
 * Copyright (C) 2020-2023 by Zach Caldwell         *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#include "Sprite.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <assert.h>
#include <physfs.h>

#include "Texture.h"
#include "Window.h"
#include "resources/epic_png_zip.h"

int main(int argc, char *argv[]) {
  // Initialization and basic sanity checks
  PHYSFS_init(argv[0]);
  assert(PHYSFS_isInit());
  assert(PHYSFS_mountMemory(epicArchive, epicArchiveLen, NULL, epicArchiveName,
                            NULL, 1));
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  renity::Window window;
  window.useFullscreen(false, false);
  window.size(renity::Dimension2Di(160, 160));
  assert(window.open() && window.isOpen());
  renity::Texture tex(window, epicFile);
  assert(tex.isValid());

  // Check constructors and setTexture
  renity::Sprite defSprite;
  assert(!defSprite.draw());
  defSprite.setTexture(&tex);
  assert(defSprite.draw());
  defSprite.setTexture(nullptr);
  assert(!defSprite.draw());
  renity::Sprite sprite(tex);
  assert(sprite.draw());
  assert(window.update());

  // Check image clipping
  renity::Rect2Di clip = sprite.getImageClip();
  assert(0 == clip.x());
  assert(0 == clip.y());
  assert(epicWidth == clip.width());
  assert(epicHeight == clip.width());
  sprite.setImageClip(renity::Rect2Di(10, 20, epicWidth + 10, epicWidth + 20));
  clip = sprite.getImageClip();
  assert(10 == clip.x());
  assert(20 == clip.y());
  assert(epicWidth + 10 == clip.width());
  assert(epicHeight + 20 == clip.height());

  // Check image rotational origin
  renity::Point2Di origin = sprite.getImageOrigin();
  // printf("(%i, %i) -> (%i, %i), (%i, %i)\n", epicWidth / 2, epicHeight / 2,
  // origin.x(), origin.y(), clip.width(), clip.height());
  // fflush(NULL);
  assert(origin.x() == epicWidth / 2);
  assert(origin.y() == epicHeight / 2);
  sprite.setImageOrigin(renity::Point2Di(17, 25));
  origin = sprite.getImageOrigin();
  assert(origin.x() == 17);
  assert(origin.y() == 25);
  sprite.useDefaultOrigin();
  origin = sprite.getImageOrigin();
  assert(origin.x() == epicWidth / 2);
  assert(origin.y() == epicHeight / 2);

  // Check image scaling
  renity::Dimension2Dd scale = sprite.getImageScale();
  assert(1.0 == scale.width());
  assert(1.0 == scale.height());
  scale.width(2.0);
  scale.height(3.0);
  sprite.setImageScale(scale);
  scale = sprite.getImageScale();
  assert(2.0 == scale.width());
  assert(3.0 == scale.height());

  // Check image rotation
  assert(0 == (Uint32)sprite.getImageRotation());
  sprite.setImageRotation(45.4);
  assert(45 == (Uint32)sprite.getImageRotation());

  // Check position get/set
  renity::Point2Di position = sprite.getPosition();
  assert(0 == position.x());
  assert(0 == position.y());
  sprite.setPosition(renity::Point2Di(15, 20));
  position = sprite.getPosition();
  assert(15 == position.x());
  assert(20 == position.y());

  // Check heading get/set
  assert(0 == (Uint32)sprite.getMoveHeading());
  sprite.setMoveHeading(180.0);
  assert(180 == (Uint32)sprite.getMoveHeading());

  // Check direction get/set
  renity::Point2Dd direction = sprite.getMoveDirection();
  assert(-0.01 < direction.x() && 0.01 > direction.x());
  assert(-1.01 < direction.y() && 1.01 > direction.y());

  // Check heading/direction interchangeability

  // Check speed get/set

  // Check bounces

  // Check move()

  // Display test
  renity::Rect2Di destTop(41, 0, epicWidth, epicHeight);
  renity::Rect2Di destBottom(41, epicHeight + 2, epicWidth, epicHeight);
  for (Uint8 i = 0; i < 30; ++i) {
    sprite.move();
    assert(sprite.draw());
    SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 255,
                           SDL_ALPHA_OPAQUE);  // Pure blue
    window.update();
    SDL_Delay(50);
  }

  // Clean up
  tex.unload();
  window.close();
  SDL_Quit();
  PHYSFS_deinit();

  return 0;
}
