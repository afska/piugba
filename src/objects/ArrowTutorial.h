#ifndef ARROW_TUTORIAL_H
#define ARROW_TUTORIAL_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "ArrowHolder.h"
#include "ArrowInfo.h"

class ArrowTutorial {
 public:
  ArrowDirection direction;

  ArrowTutorial(ArrowDirection direction);

  inline void on() { isOn = true; }
  inline void off() { isOn = false; }

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  bool isOn = false;
};

#endif  // ARROW_TUTORIAL_H