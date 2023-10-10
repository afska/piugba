#include "ArrowSelector.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 AUTOFIRE_SWITCH[] = {50, 100, 150, 200};
const u32 AUTOFIRE_DELAY[] = {25, 15, 10, 5};
const u32 AUTOFIRE_SPEEDS = 4;

ArrowSelector::ArrowSelector(ArrowDirection direction,
                             bool reuseTiles,
                             bool reactive) {
  u32 start = 0;
  ARROW_initialize(direction, start, this->flip);
  this->direction = direction;
  this->start = start;
  this->reactive = reactive;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withAnimated(start, ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(0, 0)
               .buildPtr();

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());
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
  sprite->flipHorizontally(flip == ArrowFlip::FLIP_X ||
                           flip == ArrowFlip::FLIP_BOTH);
  sprite->flipVertically(flip == ArrowFlip::FLIP_Y ||
                         flip == ArrowFlip::FLIP_BOTH);

  globalLastPressFrame++;
  currentLastPressFrame++;

  if (!reactive)
    return;

  u32 currentFrame = sprite->getCurrentFrame();
  u32 idleFrame = start + ARROW_HOLDER_IDLE(direction) + 1;
  u32 pressedFrame = start + ARROW_HOLDER_PRESSED(direction);

  if (isPressed && currentFrame != pressedFrame) {
    if (currentFrame < idleFrame || currentFrame > pressedFrame)
      SPRITE_goToFrame(sprite.get(), idleFrame);
    else
      SPRITE_goToFrame(sprite.get(), max(currentFrame + 1, pressedFrame));
  } else if (!isPressed && currentFrame >= idleFrame &&
             currentFrame <= pressedFrame)
    sprite->makeAnimated(start, ANIMATION_FRAMES, ANIMATION_DELAY);
}
