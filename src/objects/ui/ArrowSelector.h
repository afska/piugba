#ifndef ARROW_SELECTOR_H
#define ARROW_SELECTOR_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/ArrowHolder.h"
#include "objects/ArrowInfo.h"
#include "objects/base/InputHandler.h"

class ArrowSelector : public InputHandler {
 public:
  ArrowDirection direction;

  ArrowSelector(ArrowDirection direction, bool reuseTiles, bool reactive);

  bool shouldFireEvent();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  bool reactive = true;
  u32 lastPressFrame = 0;
  u32 autoFireSpeed = 1;
};

#endif  // ARROW_SELECTOR_H
