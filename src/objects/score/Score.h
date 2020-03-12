#ifndef SCORE_H
#define SCORE_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include <vector>
#include "Combo.h"
#include "ComboDigit.h"
#include "Feedback.h"

class Score {
 public:
  Score();

  void render(std::vector<Sprite*>* sprites);

 private:
  std::unique_ptr<Feedback> feedback;
  std::unique_ptr<Combo> combo;
  std::unique_ptr<ComboDigit> digit1;
  std::unique_ptr<ComboDigit> digit2;
  std::unique_ptr<ComboDigit> digit3;
};

#endif  // SCORE_H
