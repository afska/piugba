#ifndef COMBO_DIGIT_H
#define COMBO_DIGIT_H

#include <libgba-sprite-engine/sprites/sprite.h>

class ComboDigit {
 public:
  ComboDigit(u32 value, u32 position); // TODO: Move value to initialize

  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // COMBO_DIGIT_H
