#ifndef NUMERIC_PROGRESS_H
#define NUMERIC_PROGRESS_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "Of.h"
#include "objects/Digit.h"

class NumericProgress {
 public:
  NumericProgress(u32 x, u32 y);

  void setValue(u32 current, u32 total);

  void render(std::vector<Sprite*>* sprites);

  ~NumericProgress();

 private:
  std::vector<std::unique_ptr<Digit>> completedDigits;
  std::vector<std::unique_ptr<Digit>> totalDigits;
  std::unique_ptr<Of> of;
};

#endif  // NUMERIC_PROGRESS_H
