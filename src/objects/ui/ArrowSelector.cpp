#include "ArrowSelector.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 AUTOFIRE_FIRST_DELAY = 30;
const u32 AUTOFIRE_SECOND_DELAY = 10;

ArrowSelector::ArrowSelector(ArrowDirection direction,
                             bool reuseTiles,
                             bool reactive) {
  u32 start = 0;
  bool flip = false;
  ARROW_initialize(direction, start, flip);
  this->direction = direction;
  this->start = start;
  this->flip = flip;
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
  if (hasBeenPressedNow()) {
    lastPressFrame = 0;
    autoFireSpeed = 1;
  }

  if (autoFireSpeed == 1) {
    if (getIsPressed() && lastPressFrame > AUTOFIRE_FIRST_DELAY) {
      lastPressFrame = 0;
      autoFireSpeed = 2;
      return true;
    }
  } else if (autoFireSpeed == 2) {
    if (getIsPressed() && lastPressFrame > AUTOFIRE_SECOND_DELAY) {
      lastPressFrame = 0;
      return true;
    }
  }

  return hasBeenPressedNow();
}

void ArrowSelector::tick() {
  sprite->flipHorizontally(flip);
  isNewPressEvent = false;
  lastPressFrame++;

  if (!reactive)
    return;

  u32 currentFrame = sprite->getCurrentFrame();

  if (isPressed && currentFrame < start + ARROW_HOLDER_PRESSED) {
    SPRITE_goToFrame(sprite.get(),
                     max(currentFrame + 1, start + ARROW_HOLDER_IDLE + 1));
  } else if (!isPressed && currentFrame >= start + ARROW_HOLDER_IDLE) {
    sprite->makeAnimated(start, ANIMATION_FRAMES, ANIMATION_DELAY);
  }
}