#include "ArrowTutorial.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

ArrowTutorial::ArrowTutorial(ArrowDirection direction) {
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
                             ARROW_FINAL_Y)
               .buildPtr();

  SPRITE_reuseTiles(sprite.get());
}

void ArrowTutorial::tick() {
  sprite->flipHorizontally(flip);
  SPRITE_goToFrame(sprite.get(), isOn ? start : start + ARROW_HOLDER_IDLE);
}
