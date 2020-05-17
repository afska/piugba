#include "Arrow.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 HOLD_FILL_TILE = 9;
const u32 HOLD_TAIL_TILE = 0;
const u32 END_ANIMATION_START = 5;
const u32 END_ANIMATION_DELAY_FRAMES = 2;
const u32 SNAP_THRESHOLD_MS = 20;

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

void Arrow::initialize(ArrowType type,
                       ArrowDirection direction,
                       int timestamp) {
  bool isHoldFill = type == ArrowType::HOLD_FILL;
  bool isHoldTail = type == ArrowType::HOLD_TAIL;
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
    u32 tileOffset = isHoldFill ? HOLD_FILL_TILE : HOLD_TAIL_TILE;
    SPRITE_goToFrame(sprite.get(), start + tileOffset);
  } else if (isHoldFakeHead)
    animatePress();
  else
    sprite->makeAnimated(this->start, ANIMATION_FRAMES, ANIMATION_DELAY);

  parentTimestamp = 0;
  parentOffsetY = 0;
  siblingId = -1;
  partialResult = FeedbackType::UNKNOWN;
  hasEnded = false;
  endAnimationFrame = 0;
  isPressed = false;
  needsAnimation = false;

  refresh();
}

void Arrow::initialize(ArrowType type,
                       ArrowDirection direction,
                       int parentTimestamp,
                       int parentOffsetY) {
  initialize(type, direction, 0);

  parentTimestamp = parentTimestamp;
  parentOffsetY = parentOffsetY;
}

void Arrow::discard() {
  SPRITE_hide(get());

  sprite->enabled = false;
  refresh();
}

void Arrow::scheduleDiscard() {
  SPRITE_hide(get());

  isPressed = true;
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
  if (sprite->getY() <= (int)ARROW_FINAL_Y)
    animatePress();
  else {
    markAsPressed();
    needsAnimation = true;
  }
}

bool Arrow::isAligned(TimingProvider* timingProvider) {
  return abs(timingProvider->getMsecs() - (int)timestamp) < SNAP_THRESHOLD_MS;
}

ArrowState Arrow::tick(TimingProvider* timingProvider,
                       int newY,
                       bool isPressing) {
  sprite->flipHorizontally(flip);

  if (SPRITE_isHidden(sprite.get()))
    return ArrowState::OUT;

  if (type == ArrowType::HOLD_FAKE_HEAD || hasEnded) {
    endAnimationFrame++;

    if (endAnimationFrame >= END_ANIMATION_DELAY_FRAMES) {
      if (endAnimationFrame == END_ANIMATION_DELAY_FRAMES * 1)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 1);
      else if (endAnimationFrame == END_ANIMATION_DELAY_FRAMES * 2)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 2);
      else if (endAnimationFrame == END_ANIMATION_DELAY_FRAMES * 3)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 3);
      else if (endAnimationFrame == END_ANIMATION_DELAY_FRAMES * 4)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 2);
      else if (endAnimationFrame == END_ANIMATION_DELAY_FRAMES * 5)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 1);
      else if (endAnimationFrame > END_ANIMATION_DELAY_FRAMES * 5) {
        if (type == ArrowType::HOLD_FAKE_HEAD)
          animatePress();
        else
          end();
      }
    }
  } else if (isAligned(timingProvider) && isPressed && needsAnimation) {
    animatePress();
  } else if ((type == ArrowType::HOLD_HEAD || type == ArrowType::HOLD_TAIL) &&
             get()->getY() <= (int)ARROW_FINAL_Y && isPressing) {
    end();
  } else if (type == ArrowType::HOLD_FILL && isAligned(timingProvider) &&
             isPressing) {
    end();
  } else if (sprite->getY() < ARROW_OFFSCREEN_LIMIT) {
    end();
  } else
    sprite->moveTo(sprite->getX(), newY);

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

  hasEnded = true;
  endAnimationFrame = 0;
  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                 ARROW_FINAL_Y);
  SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START);
}
