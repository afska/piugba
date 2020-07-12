#ifndef DIGIT_H
#define DIGIT_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/AnimatedIndicator.h"

enum DigitSize { BIG, MINI };

class Digit : public AnimatedIndicator {
 public:
  Digit(DigitSize size, u32 x, u32 y, u32 index);

  void set(u32 value, bool isRed);

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // DIGIT_H
