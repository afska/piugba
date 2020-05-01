#ifndef ARROW_SELECTOR_H
#define ARROW_SELECTOR_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "Arrow.h"
#include "ArrowHolder.h"
#include "base/InputHandler.h"

class ArrowSelector : public InputHandler {
 public:
  ArrowDirection direction;

  ArrowSelector(ArrowDirection direction);

  void tick();
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
};

#endif  // ARROW_SELECTOR_H
