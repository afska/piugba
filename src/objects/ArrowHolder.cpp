#include "ArrowHolder.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "utils/SpriteUtils.h"

ArrowHolder::ArrowHolder(ArrowDirection direction) {
  u32 start = 0;
  bool flip = false;
  ARROW_initialize(direction, start, flip);
  this->direction = direction;
  this->start = start;
  this->flip = flip;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withSize(SIZE_16_16)
               .withLocation(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                             ARROW_CORNER_MARGIN_Y)
               .buildPtr();

  SPRITE_reuseTiles(sprite.get());
  SPRITE_goToFrame(sprite.get(), start + ARROW_HOLDER_IDLE);
}

void ArrowHolder::blink() {
  isBlinking = true;
}

void ArrowHolder::tick() {
  sprite->flipHorizontally(flip);
  isNewPressEvent = false;

  u32 currentFrame = sprite->getCurrentFrame();

  if ((isPressed || isBlinking) &&
      currentFrame < start + ARROW_HOLDER_PRESSED) {
    SPRITE_goToFrame(sprite.get(), currentFrame + 1);

    if (currentFrame + 1 == start + ARROW_HOLDER_PRESSED)
      isBlinking = false;
  } else if (!isPressed && currentFrame > start + ARROW_HOLDER_IDLE)
    SPRITE_goToFrame(sprite.get(), currentFrame - 1);
}

Sprite* ArrowHolder::get() {
  return sprite.get();
}
