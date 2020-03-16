#ifndef COMBO_DIGIT_H
#define COMBO_DIGIT_H

#include <libgba-sprite-engine/sprites/sprite.h>

class ComboDigit {
 public:
  ComboDigit(u32 position);

  void set(u32 value);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // COMBO_DIGIT_H
