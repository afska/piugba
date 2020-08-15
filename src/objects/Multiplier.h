#ifndef MULTIPLIER_H
#define MULTIPLIER_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "ArrowInfo.h"
#include "base/InputHandler.h"

class Multiplier : public InputHandler {
 public:
  Multiplier();

  void change();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 value = ARROW_DEFAULT_MULTIPLIER;

  u32 getFrame() { return value - 1; }
};

#endif  // MULTIPLIER_H
