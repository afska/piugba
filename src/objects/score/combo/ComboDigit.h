#ifndef COMBO_DIGIT_H
#define COMBO_DIGIT_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/AnimatedIndicator.h"

class ComboDigit : public AnimatedIndicator {
 public:
  ComboDigit(u32 position);

  void set(u32 value, bool isRed);

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // COMBO_DIGIT_H
