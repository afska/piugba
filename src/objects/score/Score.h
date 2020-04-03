#ifndef SCORE_H
#define SCORE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "Feedback.h"
#include "combo/Combo.h"

class Score {
 public:
  Score();

  void update(FeedbackType feedbackType);

  void tick();
  void render(std::vector<Sprite*>* sprites);

 private:
  std::unique_ptr<Feedback> feedback;
  std::unique_ptr<Combo> combo;

  int life = 60;
  std::array<u32, FEEDBACK_TYPES_TOTAL> counters;
  u32 maxCombo = 0;
  u32 points = 0;
};

#endif  // SCORE_H
