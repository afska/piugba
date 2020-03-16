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
};

#endif  // SCORE_H
