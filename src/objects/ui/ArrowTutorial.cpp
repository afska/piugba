#include "ArrowTutorial.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "utils/SpriteUtils.h"

ArrowTutorial::ArrowTutorial(ArrowDirection direction) {
  u32 start = 0;
  ARROW_initialize(direction, start, this->flip);
  this->direction = direction;
  this->start = start;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withLocation(0, 0)
               .buildPtr();

  SPRITE_reuseTiles(sprite.get());
}

void ArrowTutorial::tick() {
  sprite->flipHorizontally(flip == ArrowFlip::FLIP_X ||
                           flip == ArrowFlip::FLIP_BOTH);
  sprite->flipVertically(flip == ArrowFlip::FLIP_Y ||
                         flip == ArrowFlip::FLIP_BOTH);

  SPRITE_goToFrame(sprite.get(),
                   isOn ? start : start + ARROW_HOLDER_IDLE(direction));
}
