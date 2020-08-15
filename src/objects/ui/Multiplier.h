#ifndef MULTIPLIER_H
#define MULTIPLIER_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/InputHandler.h"

class Multiplier : public InputHandler {
 public:
  Multiplier(u32 initialValue);

  u32 change();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 value;

  u32 getFrame() { return value - 1; }
};

#endif  // MULTIPLIER_H
