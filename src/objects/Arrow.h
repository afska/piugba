#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "score/Feedback.h"
#include "utils/pool/IPoolable.h"

#define IFTEST if (false)

enum ArrowState { ACTIVE, OUT };
enum ArrowType { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_FRAMES = 9;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_CORNER_MARGIN_Y = 15;
const u32 ARROW_MARGIN = 16 + 2;
const u32 ARROW_SPEED = 3;

class Arrow : public IPoolable {
 public:
  u32 id = 0;
  ArrowType type = ArrowType::DOWNLEFT;

  Arrow(u32 id);

  void discard() override;

  void initialize(ArrowType type);
  void setNextId(int nextId);
  void press();
  bool getIsPressed();
  void markAsPressed();

  ArrowState tick(u32 msecs);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  int nextId = -1;
  u32 msecs = 0;
  u32 endTime = 0;
  bool isPressed = false;
  bool needsAnimation = false;

  void end();
  void animatePress();
  bool isAligned();
  bool isShowingPressAnimation();
};

#endif  // ARROW_H
