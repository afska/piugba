#include "Arrow.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 HOLD_FILL_TILE = 9;
const u32 HOLD_TAIL_TILE = 4;
const u32 END_ANIMATION_START = 5;
const u32 END_ANIMATION_DELAY_MS = 30;

Arrow::Arrow(u32 id) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withAnimated(ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();
  sprite->enabled = false;

  if (id != ARROW_TILEMAP_LOADING_ID)
    SPRITE_reuseTiles(sprite.get());

  this->id = id;
}

void Arrow::discard() {
  SPRITE_hide(get());
  sprite->enabled = false;
}

void Arrow::initialize(ArrowType type, ArrowDirection direction) {
  bool isHoldFill = type == ArrowType::HOLD_FILL;
  bool isHoldTail = type == ArrowType::HOLD_TAIL;
  bool isHoldFakeHead = type == ArrowType::HOLD_FAKE_HEAD;

  u32 start = 0;
  bool flip = false;
  ARROW_initialize(direction, start, flip);
  this->type = type;
  this->direction = direction;
  this->start = start;
  this->flip = flip;

  sprite->enabled = true;
  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                 GBA_SCREEN_HEIGHT);

  if (isHoldFill || isHoldTail) {
    u32 tileOffset = isHoldFill ? HOLD_FILL_TILE : HOLD_TAIL_TILE;
    SPRITE_goToFrame(sprite.get(), start + tileOffset);
  } else if (isHoldFakeHead)
    animatePress();
  else
    sprite->makeAnimated(this->start, ANIMATION_FRAMES, ANIMATION_DELAY);

  siblingId = -1;
  partialResult = FeedbackType::UNKNOWN;
  msecs = 0;
  endTime = 0;
  isPressed = false;
  needsAnimation = false;
}

void Arrow::setSiblingId(int siblingId) {
  this->siblingId = siblingId;
}

FeedbackType Arrow::getResult(FeedbackType partialResult,
                              ObjectPool<Arrow>* arrowPool) {
  this->partialResult = partialResult;

  FeedbackType result = partialResult;
  forAll(arrowPool, [&result](Arrow* sibling) {
    result = static_cast<FeedbackType>(max(result, sibling->partialResult));
  });

  return result;
}

void Arrow::press() {
  if (sprite->getY() <= (int)ARROW_CORNER_MARGIN_Y)
    animatePress();
  else {
    markAsPressed();
    needsAnimation = true;
  }
}

bool Arrow::getIsPressed() {
  return isPressed;
}

void Arrow::markAsPressed() {
  isPressed = true;
}

ArrowState Arrow::tick(int msecs, bool hasStopped, bool isPressing) {
  this->msecs = msecs;
  sprite->flipHorizontally(flip);

  if (SPRITE_isHidden(sprite.get()))
    return ArrowState::OUT;

  if (type == ArrowType::HOLD_FAKE_HEAD || isShowingPressAnimation()) {
    u32 diff = abs(msecs - endTime);

    if (diff > END_ANIMATION_DELAY_MS) {
      if (diff < END_ANIMATION_DELAY_MS * 2)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 1);
      else if (diff < END_ANIMATION_DELAY_MS * 3)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 2);
      else if (diff < END_ANIMATION_DELAY_MS * 4)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 3);
      else if (type == ArrowType::HOLD_FAKE_HEAD)
        animatePress();
      else
        end();
    }
  } else if (isAligned(0) && isPressed && needsAnimation) {
    animatePress();
  } else if ((type == ArrowType::HOLD_HEAD || type == ArrowType::HOLD_TAIL) &&
             get()->getY() <= (int)ARROW_CORNER_MARGIN_Y && isPressing) {
    end();
  } else if (type == ArrowType::HOLD_FILL && isAligned(ARROW_SPEED) &&
             isPressing) {
    end();
  } else if (sprite->getY() < ARROW_OFFSCREEN_LIMIT) {
    end();
  } else if (!hasStopped)
    sprite->moveTo(sprite->getX(), sprite->getY() - ARROW_SPEED);

  return ArrowState::ACTIVE;
}

Sprite* Arrow::get() {
  return sprite.get();
}

void Arrow::end() {
  SPRITE_hide(sprite.get());
  sprite->stopAnimating();
}

void Arrow::animatePress() {
  markAsPressed();

  endTime = msecs;
  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                 ARROW_CORNER_MARGIN_Y);
  SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START);
}

bool Arrow::isShowingPressAnimation() {
  return endTime > 0;
}

bool Arrow::isAligned(int offset) {
  return abs(sprite->getY() - (ARROW_CORNER_MARGIN_Y + offset)) < ARROW_SPEED;
}
