#include "ArrowHolder.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

ArrowHolder::ArrowHolder(ArrowDirection direction,
                         u8 playerId,
                         bool reuseTiles) {
  u32 start = 0;
  ARROW_initialize(direction, start, this->flip);
  this->direction = direction;
  this->playerId = playerId;
  this->start = start;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withLocation(
                   ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction,
                   ARROW_FINAL_Y())
               .buildPtr();

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());

  SPRITE_goToFrame(sprite.get(), start + ARROW_HOLDER_IDLE(direction));
}

void ArrowHolder::blink() {
  isBlinking = true;
}

void ArrowHolder::tick() {
  sprite->flipHorizontally(flip == ArrowFlip::FLIP_X ||
                           flip == ArrowFlip::FLIP_BOTH);
  sprite->flipVertically(flip == ArrowFlip::FLIP_Y ||
                         flip == ArrowFlip::FLIP_BOTH);

  u32 currentFrame = sprite->getCurrentFrame();
  u32 idleFrame = start + ARROW_HOLDER_IDLE(direction);
  u32 pressedFrame = start + ARROW_HOLDER_PRESSED(direction);

  if ((isPressed || isBlinking) && currentFrame < pressedFrame) {
    SPRITE_goToFrame(sprite.get(), currentFrame + 1);

    if (currentFrame + 1 == pressedFrame)
      isBlinking = false;
  } else if (!isPressed && currentFrame > idleFrame)
    SPRITE_goToFrame(sprite.get(), currentFrame - 1);
}
