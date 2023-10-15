#ifndef ARROW_TUTORIAL_H
#define ARROW_TUTORIAL_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/ArrowHolder.h"
#include "objects/ArrowInfo.h"

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
  u32 startTile = 0;
  u32 endTile = 0;
  ArrowFlip flip = ArrowFlip::NO_FLIP;
  bool isOn = false;
};

#endif  // ARROW_TUTORIAL_H
