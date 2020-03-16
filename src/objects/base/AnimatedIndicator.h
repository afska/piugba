#ifndef ANIMATED_INDICATOR_H
#define ANIMATED_INDICATOR_H

#include <libgba-sprite-engine/gba_engine.h>

class AnimatedIndicator {
 public:
  virtual void show();
  virtual void tick();

  virtual Sprite* get();

 protected:
  u32 animationPositionX;
  u32 animationPositionY;
  s8 animationDirection = 1;

 private:
  u32 currentFrame = 0;
  u32 animationFrame = 0;
};

#endif  // ANIMATED_INDICATOR_H
