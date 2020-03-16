#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "score/Feedback.h"
#include "utils/pool/IPoolable.h";

enum ArrowType { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
const u32 ARROW_CORNER_MARGIN = 4;
const u32 ARROW_MARGIN = 16 + 2;

class Arrow : public IPoolable {
 public:
  Arrow(u32 id, ArrowType type);

  void discard() override;

  void initialize();

  FeedbackType tick(u32 millis, bool isPressed);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  ArrowType type;
  bool flip = false;
  u32 endTime = 0;
  FeedbackType feedbackType = FeedbackType::ACTIVE;
};

#endif  // ARROW_H
