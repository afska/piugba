#ifndef SPRITE_UTILS_H
#define SPRITE_UTILS_H

#include <libgba-sprite-engine/sprites/sprite.h>

class SpriteUtils {
 public:
  static void goToFrame(Sprite* sprite, int frame);
  static void reuseTiles(Sprite* sprite);
};

#endif  // SPRITE_UTILS_H
