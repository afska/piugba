#include "ArrowHolder.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "utils/SpriteUtils.h"

ArrowHolder::ArrowHolder(ArrowType type) {
  bool flip = false;
  switch (type) {
    case ArrowType::UPRIGHT:
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      flip = true;
      break;
  }

  SpriteBuilder<Sprite> builder;
  sprite = builder.withSize(SIZE_16_16)
               .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * type,
                             ARROW_CORNER_MARGIN)
               .buildPtr();

  SpriteUtils::reuseTiles(sprite.get());
  SpriteUtils::goToFrame(sprite.get(), ARROW_HOLDER_IDLE);

  this->type = type;
  this->flip = flip;
}

void ArrowHolder::tick() {
  sprite->flipHorizontally(flip);
}

Sprite* ArrowHolder::get() {
  return sprite.get();
}
