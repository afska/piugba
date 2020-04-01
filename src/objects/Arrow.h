#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "score/Feedback.h"
#include "utils/pool/IPoolable.h"

enum ArrowType { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
const u32 ARROWS_TOTAL = 5;
const u32 ARROW_FRAMES = 9;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_CORNER_MARGIN_Y = 15;
const u32 ARROW_MARGIN = 16 + 2;
const u32 ARROW_SPEED = 3;

class Arrow : public IPoolable {
 public:
  ArrowType type = ArrowType::DOWNLEFT;

  Arrow(u32 id);

  void discard() override;

  void initialize(ArrowType type);
  void press();
  bool isEnding();

  FeedbackType tick(u32 msecs, bool isPressed);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  u32 msecs = 0;
  u32 endTime = 0;
};

#endif  // ARROW_H
