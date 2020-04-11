#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "score/Feedback.h"
#include "utils/pool/ObjectPool.h"

#define TEST_MODE true  // TODO: Desactivar
#define IFTEST if (TEST_MODE)
#define IFNOTTEST if (!TEST_MODE)

enum ArrowType { UNIQUE, HOLD_HEAD, HOLD_FILL, HOLD_TAIL };
enum ArrowDirection { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_SPEED = 3;
const u32 ARROW_FRAMES = 10;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_CORNER_MARGIN_Y = 15;
const u32 ARROW_MARGIN = 16 + 2;

class Arrow : public IPoolable {
 public:
  u32 id = 0;
  ArrowType type = ArrowType::UNIQUE;
  ArrowDirection direction = ArrowDirection::DOWNLEFT;

  Arrow(u32 id);

  void discard() override;

  void initialize(ArrowType type, ArrowDirection direction);
  void setSiblingId(int siblingId);
  void forAll(ObjectPool<Arrow>* arrowPool, std::function<void(Arrow*)> func);
  FeedbackType getResult(FeedbackType partialResult,
                         ObjectPool<Arrow>* arrowPool);
  void press();
  bool getIsPressed();
  void markAsPressed();

  ArrowState tick(u32 msecs);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  int siblingId = -1;
  FeedbackType partialResult = FeedbackType::UNKNOWN;
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
