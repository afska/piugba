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
  u32 startTile = 0;
  u32 endTile = 0;
  ARROW_initialize(direction, startTile, endTile, this->flip);
  this->direction = direction;
  this->startTile = startTile;
  this->endTile = endTile;
  this->reactive = reactive;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withAnimated(startTile, ANIMATION_FRAMES, ANIMATION_DELAY)
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
  u32 idleFrame = endTile + 1;
  u32 pressedFrame = endTile + ARROW_HOLDER_PRESSED_OFFSET;

  if (isPressed && currentFrame != pressedFrame) {
    if (currentFrame < idleFrame || currentFrame > pressedFrame)
      SPRITE_goToFrame(sprite.get(), idleFrame);
    else
      SPRITE_goToFrame(sprite.get(), max(currentFrame + 1, pressedFrame));
  } else if (!isPressed && currentFrame >= idleFrame &&
             currentFrame <= pressedFrame)
    sprite->makeAnimated(startTile, ANIMATION_FRAMES, ANIMATION_DELAY);
}
