#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "gameplay/TimingProvider.h"
#include "score/Feedback.h"
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

enum ArrowType { UNIQUE, HOLD_HEAD, HOLD_FILL, HOLD_TAIL, HOLD_FAKE_HEAD };
enum ArrowDirection { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_FRAMES = 10;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_TILEMAP_LOADING_ID = 1000;

const u32 ARROW_SPEED = 4;
const u32 MIN_ARROW_SPEED = 1;
const u32 MAX_ARROW_SPEED = 4;
const u32 MAX_ARROW_PER_FRAME_DISTANCE = 5;
const u32 ARROW_SIZE = 16;
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

  void discard() override;
  void scheduleDiscard();

  void initialize(ArrowType type, ArrowDirection direction, int timestamp);
  void setSiblingId(int siblingId);

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
  bool getIsPressed();
  void markAsPressed();
  bool isAligned(TimingProvider* timingProvider);

  ArrowState tick(TimingProvider* timingProvider, int newY, bool isPressing);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 start = 0;
  bool flip = false;
  int siblingId = -1;
  FeedbackType partialResult = FeedbackType::UNKNOWN;
  bool hasEnded = false;
  u32 endAnimationFrame = 0;
  bool isPressed = false;
  bool needsAnimation = false;

  void end();
  void animatePress();

  inline void refresh() {
    sprite->update();
    oam_mem[index] = sprite->oam;
  }
};

#endif  // ARROW_H
