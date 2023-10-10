#ifndef ARROW_HOLDER_H
#define ARROW_HOLDER_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "Arrow.h"
#include "base/InputHandler.h"

#define ARROW_HOLDER_IDLE(direction)              \
  ((direction == ArrowDirection::UPLEFT ||        \
    direction == ArrowDirection::UPRIGHT ||       \
    direction == ArrowDirection::UPLEFT_DOUBLE || \
    direction == ArrowDirection::UPRIGHT_DOUBLE)  \
       ? -5                                       \
       : 5)

#define ARROW_HOLDER_PRESSED(direction) (ARROW_HOLDER_IDLE(direction) + 2)

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
  ArrowFlip flip = ArrowFlip::NO_FLIP;
  bool isBlinking = false;
};

#endif  // ARROW_HOLDER_H
