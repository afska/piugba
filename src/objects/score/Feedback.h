#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "objects/base/AnimatedIndicator.h"

const u32 FEEDBACK_TOTAL_SCORES = 5;
enum FeedbackType { PERFECT, GREAT, GOOD, BAD, MISS, ACTIVE, ENDING, ENDED };

class Feedback : public AnimatedIndicator {
 public:
  Feedback();

  void setType(FeedbackType type);
  FeedbackType getType();

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
  FeedbackType type;
};

#endif  // FEEDBACK_H
