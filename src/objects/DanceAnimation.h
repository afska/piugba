#ifndef DANCE_ANIMATION_H
#define DANCE_ANIMATION_H

#include <libgba-sprite-engine/sprites/sprite.h>

class DanceAnimation {
 public:
  DanceAnimation(u32 x, u32 y);

  void update(u32 beat);

  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  int count = -3;
  int velocity = 10;
  bool toLeft = false;
};

#endif  // DANCE_ANIMATION_H
