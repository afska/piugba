#ifndef ARROW_HOLDER_H
#define ARROW_HOLDER_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "Arrow.h"

const u32 ARROW_HOLDER_IDLE = 5;
const u32 ARROW_HOLDER_PRESSED = 6;

class ArrowHolder {
 public:
  ArrowHolder(ArrowType type);

  void update();
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  ArrowType type;
  bool flip = false;
};

#endif  // ARROW_HOLDER_H
