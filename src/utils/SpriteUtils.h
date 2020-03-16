#ifndef SPRITE_UTILS_H
#define SPRITE_UTILS_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include <libgba-sprite-engine/gba_engine.h>

#define HIDDEN_WIDTH GBA_SCREEN_WIDTH - 1
#define HIDDEN_HEIGHT GBA_SCREEN_HEIGHT - 1

class SpriteUtils {
 public:
  static void hide(Sprite* sprite);
  static void goToFrame(Sprite* sprite, int frame);
  static void reuseTiles(Sprite* sprite);
};

#endif  // SPRITE_UTILS_H
