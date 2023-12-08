#include "ArrowSelector.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "data/content/_compiled_sprites/spr_arrows_alt_keys.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 AUTOFIRE_SWITCH[] = {50, 100, 150, 200};
const u32 AUTOFIRE_DELAY[] = {25, 15, 10, 5};
const u32 AUTOFIRE_SPEEDS = 4;

ArrowSelector::ArrowSelector(ArrowDirection direction,
                             bool reuseTiles,
                             bool reactive,
                             bool canUseGBAStyle,
                             bool isVertical) {
  u32 startTile = 0;
  u32 endTile = 0;
  ARROW_initialize(direction, startTile, endTile, this->flip);
  this->direction = direction;
  this->startTile = startTile;
  this->endTile = endTile;
  this->idleFrame = endTile + 1;
  this->pressedFrame = endTile + ARROW_HOLDER_PRESSED_OFFSET;
  this->animationFrames = ANIMATION_FRAMES;
  this->reactive = reactive;
  this->canUseGBAStyle = canUseGBAStyle;
  this->isVertical = isVertical;

  setUpAltKeysIfNeeded();

  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(
              isUsingGBAStyle() ? spr_arrows_alt_keysTiles : spr_arrowsTiles,
              isUsingGBAStyle() ? sizeof(spr_arrows_alt_keysTiles)
                                : sizeof(spr_arrowsTiles))
          .withSize(SIZE_16_16)
          .withAnimated(this->startTile, this->animationFrames, ANIMATION_DELAY)
          .withLocation(0, 0)
          .buildPtr();

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());

  ARROW_setUpOrientation(sprite.get(), flip);
}

bool ArrowSelector::shouldFireEvent() {
  bool isPressed = getIsPressed();
  if (!isPressed)
    return false;

  if (hasBeenPressedNow()) {
    globalLastPressFrame = 0;
    currentLastPressFrame = 0;
    autoFireSpeed = 0;
    return true;
  }

  // autofire
  bool itsTime = currentLastPressFrame >= AUTOFIRE_DELAY[autoFireSpeed];
  bool canIncreaseSpeed = autoFireSpeed < AUTOFIRE_SPEEDS - 1;
  bool shouldIncrease = globalLastPressFrame >= AUTOFIRE_SWITCH[autoFireSpeed];
  if (itsTime)
    currentLastPressFrame = 0;
  if (canIncreaseSpeed && shouldIncrease) {
    currentLastPressFrame = 0;
    autoFireSpeed++;
  }
  return itsTime;
}

void ArrowSelector::tick() {
  globalLastPressFrame++;
  currentLastPressFrame++;

  if (!reactive)
    return;

  u32 currentFrame = sprite->getCurrentFrame();

  if (isPressed && currentFrame != pressedFrame) {
    if (currentFrame < idleFrame || currentFrame > pressedFrame)
      SPRITE_goToFrame(sprite.get(), idleFrame);
    else
      SPRITE_goToFrame(sprite.get(), max(currentFrame + 1, pressedFrame));
  } else if (!isPressed && currentFrame >= idleFrame &&
             currentFrame <= pressedFrame)
    sprite->makeAnimated(startTile, animationFrames, ANIMATION_DELAY);
}

void ArrowSelector::setUpAltKeysIfNeeded() {
  if (!isUsingGBAStyle())
    return;

  if (direction == ArrowDirection::UPLEFT) {
    this->startTile = 10;
    this->endTile = 11;
    this->idleFrame = 11;
    this->pressedFrame = 11;
    this->animationFrames = 1;
    this->flip = ArrowFlip::NO_FLIP;
  } else if (direction == ArrowDirection::UPRIGHT) {
    this->startTile = 12;
    this->endTile = 13;
    this->idleFrame = 13;
    this->pressedFrame = 13;
    this->animationFrames = 1;
    this->flip = ArrowFlip::NO_FLIP;
  } else if (direction == ArrowDirection::CENTER) {
    this->startTile = 20;
    this->endTile = 25;
    this->idleFrame = 26;
    this->pressedFrame = 28;
    this->animationFrames = 5;
    this->flip = ArrowFlip::NO_FLIP;
  } else if (direction == ArrowDirection::DOWNLEFT) {
    if (isVertical) {
      this->startTile = 0;
      this->endTile = 6;
      this->idleFrame = 6;
      this->pressedFrame = 6;
      this->flip = ArrowFlip::NO_FLIP;
    } else {
      this->startTile = 3;
      this->endTile = 7;
      this->idleFrame = 7;
      this->pressedFrame = 7;
      this->animationFrames = 3;
      this->flip = ArrowFlip::FLIP_X;
    }
    this->animationFrames = 3;
  } else if (direction == ArrowDirection::DOWNRIGHT) {
    if (isVertical) {
      this->startTile = 0;
      this->endTile = 6;
      this->idleFrame = 6;
      this->pressedFrame = 6;
      this->flip = ArrowFlip::FLIP_Y;
    } else {
      this->startTile = 3;
      this->endTile = 7;
      this->idleFrame = 7;
      this->pressedFrame = 7;
      this->flip = ArrowFlip::NO_FLIP;
    }
    this->animationFrames = 3;
  }
}
