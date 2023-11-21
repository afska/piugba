#ifndef ARROW_HOLDER_H
#define ARROW_HOLDER_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "Arrow.h"
#include "base/InputHandler.h"

#define ARROW_HOLDER_PRESSED_OFFSET 2

class ArrowHolder : public InputHandler {
 public:
  ArrowDirection direction;
  u8 playerId;

  ArrowHolder(ArrowDirection direction, u8 playerId, bool reuseTiles);

  void blink();

  void tick(int offsetX = 0);
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 startTile = 0;
  u32 endTile = 0;
  ArrowFlip flip = ArrowFlip::NO_FLIP;
  bool isBlinking = false;
};

#endif  // ARROW_HOLDER_H
