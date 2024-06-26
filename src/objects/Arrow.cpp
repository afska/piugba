#include "Arrow.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "data/content/_compiled_sprites/spr_arrows_mdrn.h"
#include "utils/SpriteUtils.h"

const u32 END_ANIMATION_DELAY_FRAMES = 2;
const u32 END_ANIMATION_TOTAL_FRAMES = 12;
const u32 END_ANIMATION_FRAMES[] = {0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 1, 1};

Arrow::Arrow(u32 id) {
  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(SAVEFILE_isUsingModernTheme() ? spr_arrows_mdrnTiles
                                                  : spr_arrowsTiles,
                    SAVEFILE_isUsingModernTheme() ? sizeof(spr_arrows_mdrnTiles)
                                                  : sizeof(spr_arrowsTiles))
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
  sprite->update();
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
  if (sprite->getY() <= (int)ARROW_FINAL_Y())
    animatePress();
  else {
    markAsPressed();
    needsAnimation = true;
  }
}

CODE_IWRAM bool Arrow::tick(int newY, bool isPressing, int offsetX) {
  if (SPRITE_isHidden(get()))
    return true;

  bool isHoldArrow = type == ArrowType::HOLD_HEAD ||
                     type == ArrowType::HOLD_TAIL ||
                     type == ArrowType::HOLD_FILL;
  bool isHoldFill = type == ArrowType::HOLD_FILL;
  bool isFakeHead = type == ArrowType::HOLD_FAKE_HEAD;
  int newX =
      ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction + offsetX;
  bool $isNearEndOrClose = isNearEndOrClose(newY);

  if (isFakeHead || hasEnded) {
    endAnimationFrame++;
    sprite->moveTo(newX, sprite->getY());

    if (endAnimationFrame < END_ANIMATION_TOTAL_FRAMES) {
      SPRITE_goToFrame(
          sprite.get(),
          endAnimationStartFrame + END_ANIMATION_FRAMES[endAnimationFrame]);
    } else {
      if (type == ArrowType::HOLD_FAKE_HEAD)
        animatePress();
      else
        return end();
    }
  } else if ($isNearEndOrClose && needsAnimation) {
    animatePress();
  } else if (isHoldArrow && (!isHoldFill || isLastFill) && isNearEnd(newY) &&
             isPressing) {
    if (!isHoldFill)
      markAsPressed();

    return end();
  } else if (sprite->getY() < ARROW_OFFSCREEN_LIMIT) {
    return end();
  } else
    sprite->moveTo(newX, newY);

  return $isNearEndOrClose;
}

bool Arrow::end() {
  SPRITE_hide(sprite.get());
  sprite->stopAnimating();

  if (type == ArrowType::HOLD_FILL)
    holdArrow->activeFillCount--;

  return true;
}

void Arrow::animatePress() {
  markAsPressed();

  hasEnded = true;
  endAnimationFrame = 0;
  sprite->moveTo(ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction,
                 ARROW_FINAL_Y());
  SPRITE_goToFrame(sprite.get(), endAnimationStartFrame);
}

bool Arrow::isNearEndOrClose(int newY) {
  return newY <= (int)ARROW_FINAL_Y() + (int)ARROW_QUARTER_SIZE;
}

bool Arrow::isNearEnd(int newY) {
  return newY <= (int)ARROW_FINAL_Y();
}
