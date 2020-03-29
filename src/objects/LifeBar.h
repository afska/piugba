#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/sprites/sprite.h>

class LifeBar {
 public:
  LifeBar();

  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // LIFE_BAR_H
