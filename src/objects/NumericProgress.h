#ifndef NUMERIC_PROGRESS_H
#define NUMERIC_PROGRESS_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "score/combo/ComboDigit.h"

class NumericProgress {
 public:
  NumericProgress();

  void setValue(u32 current, u32 total);

  void render(std::vector<Sprite*>* sprites);

  ~NumericProgress();

 private:
  std::vector<std::unique_ptr<ComboDigit>> digits;
};

#endif  // NUMERIC_PROGRESS_H
