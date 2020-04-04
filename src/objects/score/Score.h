#ifndef SCORE_H
#define SCORE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "Feedback.h"
#include "combo/Combo.h"
#include "objects/LifeBar.h"

class Score {
 public:
  Score(LifeBar* lifeBar);

  void update(FeedbackType feedbackType);

  void tick();
  void render(std::vector<Sprite*>* sprites);

 private:
  std::unique_ptr<Feedback> feedback;
  std::unique_ptr<Combo> combo;
  LifeBar* lifeBar;

  bool hasMissCombo = false;
  u32 maxCombo = 0;
  int life = INITIAL_LIFE;
  std::array<u32, FEEDBACK_TYPES_TOTAL> counters;
  u32 points = 0;

  void updateCombo(FeedbackType feedbackType);
  void updateLife(FeedbackType feedbackType);
  void updateCounters(FeedbackType feedbackType);
  void updatePoints(FeedbackType feedbackType);
};

#endif  // SCORE_H
