#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "gameplay/TimingProvider.h"
#include "score/Feedback.h"
#include "utils/SpriteUtils.h"
#include "utils/pool/ObjectPool.h"

// TEST MACROS
#define TEST_MODE true
#define KEYTEST_MODE false
#define TIMINGTEST_MODE false
#define IFTEST if (TEST_MODE)
#define IFNOTTEST if (!TEST_MODE)
#define IFKEYTEST if (KEYTEST_MODE)
#define IFTIMINGTEST if (TIMINGTEST_MODE)
#define IFNOTKEYTEST if (!KEYTEST_MODE)
#define IFNOTTIMINGTEST if (!TIMINGTEST_MODE)
#define DEBULOG(NUM) LOGN(NUM, -1);
#define LOGN(NUM, LINE) (LOGSTR(std::to_string(NUM).c_str(), LINE))
#define LOGSTR(STR, LINE) (TextStream::instance().setText(STR, 1 + LINE, 15))
#include <libgba-sprite-engine/background/text_stream.h>

class HoldArrow;

enum ArrowType {
  UNIQUE,
  HOLD_HEAD,
  HOLD_FILL,
  HOLD_TAIL_EXTRA_FILL,
  HOLD_TAIL_ARROW,
  HOLD_FAKE_HEAD
};
enum ArrowDirection { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_FRAMES = 10;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_TILEMAP_LOADING_ID = 1000;
const u32 ARROW_ANIMATION_FRAMES = 5;
const u32 ARROW_ANIMATION_DELAY = 2;
const u32 ARROW_HOLD_FILL_TILE = 9;
const u32 ARROW_HOLD_TAIL_TILE = 0;

const u32 ARROW_DEFAULT_MULTIPLIER = 3;
const u32 ARROW_MIN_MULTIPLIER = 1;
const u32 ARROW_MAX_MULTIPLIER = 4;
const u32 ARROW_SIZE = 16;
const u32 ARROW_QUARTER_SIZE = 4;
const u32 ARROW_MARGIN = ARROW_SIZE + 2;
const u32 ARROW_INITIAL_Y = GBA_SCREEN_HEIGHT;
const u32 ARROW_FINAL_Y = 15;
const u32 ARROW_DISTANCE = ARROW_INITIAL_Y - ARROW_FINAL_Y;

inline void ARROW_initialize(ArrowDirection direction, u32& start, bool& flip) {
  switch (direction) {
    case ArrowDirection::DOWNLEFT:
      start = ARROW_FRAMES * 0;
      break;
    case ArrowDirection::UPLEFT:
      start = ARROW_FRAMES * 1;
      break;
    case ArrowDirection::CENTER:
      start = ARROW_FRAMES * 2;
      break;
    case ArrowDirection::UPRIGHT:
      start = ARROW_FRAMES * 1;
      flip = true;
      break;
    case ArrowDirection::DOWNRIGHT:
      start = ARROW_FRAMES * 0;
      flip = true;
      break;
  }
}

class Arrow : public IPoolable {
 public:
  u32 id = 0;
  ArrowType type = ArrowType::UNIQUE;
  ArrowDirection direction = ArrowDirection::DOWNLEFT;
  int timestamp = 0;
  u32 index = 0;

  Arrow(u32 id);

  inline void initialize(ArrowType type,
                         ArrowDirection direction,
                         int timestamp) {
    bool isHoldFill =
        type == ArrowType::HOLD_FILL || type == ArrowType::HOLD_TAIL_EXTRA_FILL;
    bool isHoldTail = type == ArrowType::HOLD_TAIL_ARROW;
    bool isHoldFakeHead = type == ArrowType::HOLD_FAKE_HEAD;

    u32 start = 0;
    bool flip = false;
    ARROW_initialize(direction, start, flip);
    this->type = type;
    this->direction = direction;
    this->timestamp = timestamp;
    this->start = start;
    this->flip = flip;

    sprite->enabled = true;
    sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                   ARROW_INITIAL_Y);

    if (isHoldFill || isHoldTail) {
      u32 tileOffset = isHoldFill ? ARROW_HOLD_FILL_TILE : ARROW_HOLD_TAIL_TILE;
      SPRITE_goToFrame(sprite.get(), start + tileOffset);
    } else if (isHoldFakeHead)
      animatePress();
    else
      sprite->makeAnimated(this->start, ARROW_ANIMATION_FRAMES,
                           ARROW_ANIMATION_DELAY);

    siblingId = -1;
    holdArrow = NULL;
    holdStartTime = 0;
    holdEndTime = 0;
    fillIndex = -1;
    previousFill = NULL;
    partialResult = FeedbackType::UNKNOWN;
    hasEnded = false;
    endAnimationFrame = 0;
    isPressed = false;
    needsAnimation = false;

    refresh();
  }

  inline void initializeHoldBorder(ArrowType type,
                                   ArrowDirection direction,
                                   int timestamp,
                                   HoldArrow* holdArrow) {
    initialize(type, direction, timestamp);
    setHoldArrow(holdArrow);
  }

  inline void initializeHoldFill(ArrowDirection direction,
                                 HoldArrow* holdArrow,
                                 int fillIndex) {
    initialize(ArrowType::HOLD_FILL, direction, timestamp);
    setHoldArrow(holdArrow);
    this->fillIndex = fillIndex;
  }

  void discard() override;
  void scheduleDiscard();

  inline void setSiblingId(int siblingId) { this->siblingId = siblingId; }

  inline HoldArrow* getHoldArrow() {
    return isHoldArrowAlive() ? holdArrow : NULL;
  }
  inline int getHoldStartTime() { return holdStartTime; }
  inline int getHoldEndTime() { return holdEndTime; }
  inline int getFillIndex() { return fillIndex; }

  template <typename F>
  inline void forAll(ObjectPool<Arrow>* arrowPool, F func) {
    func(this);

    if (siblingId < 0)
      return;

    u32 currentId = siblingId;
    do {
      Arrow* current = arrowPool->getByIndex(currentId);
      currentId = current->siblingId;
      func(current);
    } while (currentId != id);
  }

  FeedbackType getResult(FeedbackType partialResult,
                         ObjectPool<Arrow>* arrowPool);
  void press();
  inline bool getIsPressed() { return isPressed; }
  inline void markAsPressed() { isPressed = true; }

  ArrowState tick(int newY, bool isPressing);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  HoldArrow* holdArrow = NULL;
  int holdStartTime = 0;
  int holdEndTime = 0;
  int siblingId = -1;
  int fillIndex = -1;
  Arrow* previousFill = NULL;
  FeedbackType partialResult = FeedbackType::UNKNOWN;
  bool hasEnded = false;
  u32 endAnimationFrame = 0;
  bool isPressed = false;
  bool needsAnimation = false;

  void end();
  void animatePress();
  bool isAligned();
  bool isNearEnd();

  void setHoldArrow(HoldArrow* holdArrow);
  bool isHoldArrowAlive();

  inline void refresh() {
    sprite->update();
    oam_mem[index] = sprite->oam;
  }
};

#endif  // ARROW_H
