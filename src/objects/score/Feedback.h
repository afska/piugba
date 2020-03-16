#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <libgba-sprite-engine/sprites/sprite.h>

enum FeedbackType { PERFECT, GREAT, GOOD, BAD, MISS, ACTIVE };

class Feedback {
 public:
  Feedback(FeedbackType type); // TODO: MOVE TO INITIALIZE

  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  FeedbackType type;
};

#endif  // FEEDBACK_H
