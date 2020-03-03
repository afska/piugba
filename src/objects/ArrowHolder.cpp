#include "ArrowHolder.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/spr_arrow_center.h"
#include "data/spr_arrow_center_placeholder.h"
#include "data/spr_arrow_downleft.h"
#include "data/spr_arrow_downleft_placeholder.h"
#include "data/spr_arrow_upleft.h"
#include "data/spr_arrow_upleft_placeholder.h"

ArrowHolder::ArrowHolder(ArrowType type) {
  const unsigned int* tiles;
  int size;
  bool flip = false;
  switch (type) {
    case ArrowType::DOWNLEFT:
      tiles = spr_arrow_downleft_placeholderTiles;
      size = spr_arrow_downleft_placeholderTilesLen;
      break;
    case ArrowType::UPLEFT:
      tiles = spr_arrow_upleft_placeholderTiles;
      size = spr_arrow_upleft_placeholderTilesLen;
      break;
    case ArrowType::CENTER:
      tiles = spr_arrow_center_placeholderTiles;
      size = spr_arrow_center_placeholderTilesLen;
      break;
    case ArrowType::UPRIGHT:
      tiles = spr_arrow_upleft_placeholderTiles;
      size = spr_arrow_upleft_placeholderTilesLen;
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      tiles = spr_arrow_downleft_placeholderTiles;
      size = spr_arrow_downleft_placeholderTilesLen;
      flip = true;
      break;
  }

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(tiles, size)
               .withSize(SIZE_16_16)
               .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * type,
                             ARROW_CORNER_MARGIN)
               .buildPtr();
  this->type = type;
  this->flip = flip;
}

void ArrowHolder::update() {
  sprite->flipHorizontally(flip);
}

Sprite* ArrowHolder::get() {
  return sprite.get();
}
