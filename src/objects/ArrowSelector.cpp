#include "ArrowSelector.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_arrows.h"
#include "utils/SpriteUtils.h"

ArrowSelector::ArrowSelector(ArrowDirection direction) {
  u32 start = 0;
  bool flip = false;
  ARROW_initialize(direction, start, flip);
  this->direction = direction;
  this->start = start;
  this->flip = flip;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withLocation(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                             ARROW_CORNER_MARGIN_Y)
               .buildPtr();

  if (direction > 0)
    SPRITE_reuseTiles(sprite.get());
  SPRITE_goToFrame(sprite.get(), start + ARROW_HOLDER_IDLE);
}

void ArrowSelector::tick() {
  sprite->flipHorizontally(flip);
  isNewPressEvent = false;

  u32 currentFrame = sprite->getCurrentFrame();
  if (isPressed && currentFrame < start + ARROW_HOLDER_PRESSED) {
    SPRITE_goToFrame(sprite.get(), currentFrame + 1);
  } else if (!isPressed && currentFrame > start + ARROW_HOLDER_IDLE)
    SPRITE_goToFrame(sprite.get(), currentFrame - 1);
}

Sprite* ArrowSelector::get() {
  return sprite.get();
}
