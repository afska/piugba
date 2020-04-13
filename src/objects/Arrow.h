#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "score/Feedback.h"
#include "utils/pool/ObjectPool.h"

// TEST MACROS
#define TEST_MODE true  // TODO: REMOVE
#define KEYTEST_MODE true
#define IFTEST if (TEST_MODE)
#define IFNOTTEST if (!TEST_MODE)
#define IFKEYTEST if (KEYTEST_MODE)
#define IFNOTKEYTEST if (!KEYTEST_MODE)
#define LOG(NUM) \
  (TextStream::instance().setText(std::to_string(NUM).c_str(), 0, 15))
#include <libgba-sprite-engine/background/text_stream.h>

enum ArrowType { UNIQUE, HOLD_HEAD, HOLD_FILL, HOLD_TAIL, HOLD_FAKE_HEAD };
enum ArrowDirection { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_SPEED = 3;
const u32 ARROW_FRAMES = 10;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_CORNER_MARGIN_Y = 15;
const u32 ARROW_HEIGHT = 16;
const u32 ARROW_MARGIN = ARROW_HEIGHT + 2;
const u32 ARROW_TILEMAP_LOADING_ID = 1000;

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

  ArrowState tick(u32 msecs, bool hasStopped, bool isPressing);
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
  bool isAligned(int offset);
  bool isShowingPressAnimation();
};

#endif  // ARROW_H
