#include "ArrowHolder.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "utils/SpriteUtils.h"

ArrowHolder::ArrowHolder(ArrowDirection direction) {
  int start = 0;
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

  SpriteBuilder<Sprite> builder;
  sprite = builder.withSize(SIZE_16_16)
               .withLocation(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                             ARROW_CORNER_MARGIN_Y)
               .buildPtr();

  SPRITE_reuseTiles(sprite.get());
  SPRITE_goToFrame(sprite.get(), start + ARROW_HOLDER_IDLE);

  this->direction = direction;
  this->start = start;
  this->flip = flip;
}

void ArrowHolder::blink() {
  isBlinking = true;
}

bool ArrowHolder::getIsPressed() {
  return isPressed;
}

bool ArrowHolder::hasBeenPressedNow() {
  return isNewPressEvent;
}

void ArrowHolder::setIsPressed(bool isPressed) {
  bool isNewPressEvent = !this->isPressed && isPressed;
  this->isPressed = isPressed;

  this->isNewPressEvent = isNewPressEvent;
  IFTEST this->isNewPressEvent = isPressed;
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
