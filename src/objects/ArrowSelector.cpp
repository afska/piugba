#include "ArrowSelector.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;

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
               .withAnimated(start, ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * direction,
                             ARROW_FINAL_Y)
               .buildPtr();

  if (direction > 0)
    SPRITE_reuseTiles(sprite.get());
}

void ArrowSelector::tick() {
  sprite->flipHorizontally(flip);
  isNewPressEvent = false;

  u32 currentFrame = sprite->getCurrentFrame();

  if (isPressed && currentFrame < start + ARROW_HOLDER_PRESSED) {
    SPRITE_goToFrame(sprite.get(),
                     max(currentFrame + 1, start + ARROW_HOLDER_IDLE + 1));
  } else if (!isPressed && currentFrame >= start + ARROW_HOLDER_IDLE) {
    sprite->makeAnimated(this->start, ANIMATION_FRAMES, ANIMATION_DELAY);
  }
}
