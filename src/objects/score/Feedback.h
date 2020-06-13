#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/AnimatedIndicator.h"

enum FeedbackType { PERFECT, GREAT, GOOD, BAD, MISS, UNKNOWN };

const int FEEDBACK_TYPES_TOTAL = 5;

class Feedback : public AnimatedIndicator {
 public:
  Feedback();

  void setType(FeedbackType type);
  inline FeedbackType getType() { return this->type; }

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
  FeedbackType type;
};

#endif  // FEEDBACK_H
