#ifndef COMBO_H
#define COMBO_H

#include <libgba-sprite-engine/sprites/sprite.h>

class Combo {
 public:
  Combo();

  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // COMBO_H
