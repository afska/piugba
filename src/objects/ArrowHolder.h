#ifndef ARROW_HOLDER_H
#define ARROW_HOLDER_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "Arrow.h"
#include "base/InputHandler.h"

const u32 ARROW_HOLDER_IDLE = 5;
const u32 ARROW_HOLDER_PRESSED = 7;

class ArrowHolder : public InputHandler {
 public:
  ArrowDirection direction;
  u8 playerId;

  ArrowHolder(ArrowDirection direction, u8 playerId, bool reuseTiles);

  void blink();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  bool isBlinking = false;
};

#endif  // ARROW_HOLDER_H
