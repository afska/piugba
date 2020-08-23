#ifndef SCORE_H
#define SCORE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <array>
#include <vector>

#include "Feedback.h"
#include "combo/Combo.h"
#include "gameplay/Evaluation.h"
#include "objects/LifeBar.h"

class Score {
 public:
  Score(LifeBar* lifeBar);

  bool update(FeedbackType feedbackType, bool isLong);
  std::unique_ptr<Evaluation> evaluate();
  void relocate();

  void tick();
  void render(std::vector<Sprite*>* sprites);

 private:
  std::unique_ptr<Feedback> feedback;
  std::unique_ptr<Combo> combo;
  LifeBar* lifeBar;

  int life = INITIAL_LIFE;
  bool hasMissCombo = false;
  u32 maxCombo = 0;
  std::array<u32, FEEDBACK_TYPES_TOTAL> counters;
  u32 points = 0;
  u32 longNotes = 0;

  bool updateLife(FeedbackType feedbackType);
  void updateCombo(FeedbackType feedbackType);
  void updateCounters(FeedbackType feedbackType);
  void updatePoints(FeedbackType feedbackType);
};

#endif  // SCORE_H
