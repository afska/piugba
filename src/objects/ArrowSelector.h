#ifndef ARROW_SELECTOR_H
#define ARROW_SELECTOR_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "ArrowHolder.h"
#include "ArrowInfo.h"
#include "base/InputHandler.h"

class ArrowSelector : public InputHandler {
 public:
  ArrowDirection direction;

  ArrowSelector(ArrowDirection direction);

  bool shouldFireEvent();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  u32 lastPressFrame = 0;
  u32 autoFireSpeed = 1;
};

#endif  // ARROW_SELECTOR_H
