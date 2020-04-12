#include "Arrow.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 HOLD_FILL_TILE = 9;
const u32 HOLD_END_TILE = 4;
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

  if (id > 0)
    SPRITE_reuseTiles(sprite.get());

  this->id = id;
}

void Arrow::discard() {
  sprite->enabled = false;
}

void Arrow::initialize(ArrowType type, ArrowDirection direction) {
  bool isHoldFill = type == ArrowType::HOLD_FILL;
  bool isHoldTail = type == ArrowType::HOLD_TAIL;

  u32 start = 0;
  bool flip = false;
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

  this->type = type;
  this->direction = direction;
  this->start = start;
  this->flip = flip;

  sprite->enabled = true;
  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                 GBA_SCREEN_HEIGHT);

  if (isHoldFill || isHoldTail) {
    u32 tileOffset = isHoldFill ? HOLD_FILL_TILE : HOLD_END_TILE;
    SPRITE_goToFrame(sprite.get(), start + tileOffset);
  } else
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

void Arrow::forAll(ObjectPool<Arrow>* arrowPool,
                   std::function<void(Arrow*)> func) {
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

ArrowState Arrow::tick(u32 msecs, bool hasStopped, bool isKeyPressed) {
  this->msecs = msecs;
  sprite->flipHorizontally(flip);

  if (SPRITE_isHidden(sprite.get()))
    return ArrowState::OUT;

  if (isShowingPressAnimation()) {
    u32 diff = abs(msecs - endTime);

    if (diff > END_ANIMATION_DELAY_MS) {
      if (diff < END_ANIMATION_DELAY_MS * 2)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 1);
      else if (diff < END_ANIMATION_DELAY_MS * 3)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 2);
      else if (diff < END_ANIMATION_DELAY_MS * 4)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 3);
      else
        end();
    }
  } else if (isAligned() && ((type != ArrowType::UNIQUE && isKeyPressed) ||
                             (isPressed && needsAnimation))) {
    animatePress();
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
  sprite->moveTo(sprite->getX(), ARROW_CORNER_MARGIN_Y);
  SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START);
}

bool Arrow::isShowingPressAnimation() {
  return endTime > 0;
}

bool Arrow::isAligned() {
  return abs(sprite->getY() - ARROW_CORNER_MARGIN_Y) < ARROW_SPEED;
}
