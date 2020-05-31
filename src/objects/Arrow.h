#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "ArrowInfo.h"
#include "gameplay/HoldArrow.h"
#include "gameplay/TimingProvider.h"
#include "score/Feedback.h"
#include "utils/SpriteUtils.h"
#include "utils/pool/ObjectPool.h"

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
    bool isHoldFill = type == ArrowType::HOLD_FILL ||
                      type == ArrowType::HOLD_HEAD_EXTRA_FILL ||
                      type == ArrowType::HOLD_TAIL_EXTRA_FILL;
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
    partialResult = FeedbackType::UNKNOWN;
    hasEnded = false;
    endAnimationFrame = 0;
    isPressed = false;
    needsAnimation = false;

    if (type == ArrowType::HOLD_HEAD_EXTRA_FILL)
      sprite->setPriority(ARROW_LAYER_MIDDLE);
    else if (isHoldTail)
      sprite->setPriority(ARROW_LAYER_BACK);
    else
      sprite->setPriority(ARROW_LAYER_FRONT);

    refresh();
  }

  inline void initializeHoldBorder(ArrowType type,
                                   ArrowDirection direction,
                                   int timestamp,
                                   HoldArrow* holdArrow) {
    initialize(type, direction, timestamp);
    this->holdArrow = holdArrow;
  }

  inline void initializeHoldFill(ArrowDirection direction,
                                 HoldArrow* holdArrow) {
    initialize(ArrowType::HOLD_FILL, direction, timestamp);
    this->holdArrow = holdArrow;
  }

  void discard() override;
  void scheduleDiscard();

  inline void setSiblingId(int siblingId) { this->siblingId = siblingId; }
  inline HoldArrow* getHoldArrow() { return holdArrow; }

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
  int siblingId = -1;
  FeedbackType partialResult = FeedbackType::UNKNOWN;
  bool hasEnded = false;
  u32 endAnimationFrame = 0;
  bool isPressed = false;
  bool needsAnimation = false;

  ArrowState end();
  void animatePress();
  bool isAligned();
  bool isNearEnd(int newY);

  inline void refresh() {
    sprite->update();
    oam_mem[index] = sprite->oam;
  }
};

#endif  // ARROW_H
