#ifndef ARROW_H
#define ARROW_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "ArrowInfo.h"
#include "gameplay/HoldArrow.h"
#include "gameplay/TimingProvider.h"
#include "score/Feedback.h"
#include "utils/SpriteUtils.h"
#include "utils/pool/ObjectPool.h"

#define EXPLOSION_ANIMATION_START 15

inline void ARROW_initialize(ArrowDirection direction,
                             u32& startTile,
                             u32& endTile,
                             ArrowFlip& flip) {
  ArrowDirection singleDirection =
      static_cast<ArrowDirection>(direction % ARROWS_TOTAL);
  startTile = ARROW_BASE_TILE[singleDirection];
  endTile = ARROW_END_TILE[singleDirection];
  flip = ARROW_FLIP_TILE[singleDirection];
}

inline void ARROW_setUpOrientation(Sprite* sprite, ArrowFlip flip) {
  sprite->flipHorizontally(flip == ArrowFlip::FLIP_X ||
                           flip == ArrowFlip::FLIP_BOTH);
  sprite->flipVertically(flip == ArrowFlip::FLIP_Y ||
                         flip == ArrowFlip::FLIP_BOTH);
}

class Arrow : public IPoolable {
 public:
  u32 id = 0;
  ArrowType type = ArrowType::UNIQUE;
  ArrowDirection direction = ArrowDirection::DOWNLEFT;
  u8 playerId;
  int timestamp = 0;
  bool isFake = false;
  u32 index = 0;

  Arrow(u32 id);

  inline void initialize(ArrowType type,
                         ArrowDirection direction,
                         u8 playerId,
                         int timestamp,
                         bool isFake = false) {
    bool isHoldFill = type == ArrowType::HOLD_FILL;
    bool isHoldTail = type == ArrowType::HOLD_TAIL;
    bool isHoldFakeHead = type == ArrowType::HOLD_FAKE_HEAD;

    u32 startTile = 0;
    u32 endTile = 0;
    ARROW_initialize(direction, startTile, endTile, this->flip);
    this->type = type;
    this->direction = direction;
    this->playerId = playerId;
    this->timestamp = timestamp;
    this->isFake = isFake;
    this->startTile = startTile;
    this->endTile = endTile;
    this->endAnimationStartFrame =
        isHoldFakeHead ? endTile : EXPLOSION_ANIMATION_START;

    sprite->enabled = true;
    sprite->moveTo(ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction,
                   ARROW_INITIAL_Y);

    if (isFake)
      SPRITE_goToFrame(sprite.get(), endTile + ARROW_FAKE_TILE);
    else if (isHoldFill || isHoldTail) {
      u32 tileOffset = isHoldFill ? ARROW_HOLD_FILL_TILE : ARROW_HOLD_TAIL_TILE;
      SPRITE_goToFrame(sprite.get(), startTile + tileOffset);
    } else if (isHoldFakeHead)
      animatePress();
    else
      sprite->makeAnimated(startTile, ARROW_ANIMATION_FRAMES,
                           ARROW_ANIMATION_DELAY);

    siblingId = -1;
    holdArrow = NULL;
    isLastFill = false;
    partialResult = FeedbackType::UNKNOWN;
    hasEnded = false;
    endAnimationFrame = 0;
    isPressed = false;
    needsAnimation = false;

    sprite->setPriority(isHoldTail ? ARROW_LAYER_BACK : ARROW_LAYER_FRONT);
    ARROW_setUpOrientation(sprite.get(), flip);
  }

  inline void initializeHoldBorder(ArrowType type,
                                   ArrowDirection direction,
                                   u8 playerId,
                                   int timestamp,
                                   HoldArrow* holdArrow,
                                   bool isFake) {
    initialize(type, direction, playerId, timestamp, isFake);
    this->holdArrow = holdArrow;
  }

  inline void initializeHoldFill(ArrowDirection direction,
                                 u8 playerId,
                                 HoldArrow* holdArrow) {
    initialize(ArrowType::HOLD_FILL, direction, playerId, timestamp,
               holdArrow->isFake);
    this->holdArrow = holdArrow;
  }

  void discard() override;
  void scheduleDiscard();

  inline void setSiblingId(int siblingId) { this->siblingId = siblingId; }
  inline HoldArrow* getHoldArrow() { return holdArrow; }
  inline void setIsLastFill(bool isLastFill) { this->isLastFill = isLastFill; }

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

  ArrowState tick(int newY, bool isPressing, int offsetX = 0);
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 startTile = 0;
  u32 endTile = 0;
  ArrowFlip flip = ArrowFlip::NO_FLIP;
  int siblingId = -1;
  HoldArrow* holdArrow = NULL;
  bool isLastFill = false;
  FeedbackType partialResult = FeedbackType::UNKNOWN;
  bool hasEnded = false;
  u32 endAnimationStartFrame = 0;
  u32 endAnimationFrame = 0;
  bool isPressed = false;
  bool needsAnimation = false;

  ArrowState end();
  void animatePress();
  bool isNearEndOrClose(int newY);
  bool isNearEnd(int newY);

  inline void refresh() {
    sprite->update();
    oam_mem[index] = sprite->oam;
  }
};

#endif  // ARROW_H
