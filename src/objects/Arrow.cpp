#include "Arrow.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 END_ANIMATION_START = 5;
const u32 END_ANIMATION_DELAY_FRAMES = 2;

Arrow::Arrow(u32 id) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withAnimated(ARROW_ANIMATION_FRAMES, ARROW_ANIMATION_DELAY)
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
  refresh();
}

void Arrow::scheduleDiscard() {
  end();
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

ArrowState Arrow::tick(int newY, bool isPressing) {
  sprite->flipHorizontally(flip);
  bool isHoldArrow = type == ArrowType::HOLD_HEAD_ARROW ||
                     type == ArrowType::HOLD_TAIL_ARROW ||
                     type == ArrowType::HOLD_FILL ||
                     type == ArrowType::HOLD_HEAD_EXTRA_FILL ||
                     type == ArrowType::HOLD_TAIL_EXTRA_FILL;

  if (SPRITE_isHidden(get()))
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
          return end();
      }
    }
  } else if (isAligned() && isPressed && needsAnimation) {
    animatePress();
  } else if (isHoldArrow && isNearEnd(newY) && isPressing) {
    return end();
  } else if (sprite->getY() < ARROW_OFFSCREEN_LIMIT) {
    return end();
  } else
    sprite->moveTo(sprite->getX(), newY);

  return ArrowState::ACTIVE;
}

Sprite* Arrow::get() {
  return sprite.get();
}

ArrowState Arrow::end() {
  SPRITE_hide(sprite.get());
  sprite->stopAnimating();

  if (type == ArrowType::HOLD_FILL && isHoldArrowAlive())
    holdArrow->activeFillCount--;

  return ArrowState::OUT;
}

void Arrow::animatePress() {
  markAsPressed();

  hasEnded = true;
  endAnimationFrame = 0;
  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                 ARROW_FINAL_Y);
  SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START);
}

bool Arrow::isAligned() {
  return abs(sprite->getY() - ARROW_FINAL_Y) < ARROW_QUARTER_SIZE;
}

bool Arrow::isNearEnd(int newY) {
  return newY <= (int)(ARROW_FINAL_Y);
}
