#include "Arrow.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/spr_arrow_center.h"
#include "data/spr_arrow_downleft.h"
#include "data/spr_arrow_upleft.h"

const int ANIMATION_FRAMES = 5;
const int ANIMATION_DELAY = 2;

Arrow::Arrow(u32 id, ArrowType type) {
  const unsigned int* tiles;
  int size;
  bool flip = false;
  switch (type) {
    case ArrowType::DOWNLEFT:
      tiles = spr_arrow_downleftTiles;
      size = spr_arrow_downleftTilesLen;
      break;
    case ArrowType::UPLEFT:
      tiles = spr_arrow_upleftTiles;
      size = spr_arrow_upleftTilesLen;
      break;
    case ArrowType::CENTER:
      tiles = spr_arrow_centerTiles;
      size = spr_arrow_centerTilesLen;
      break;
    case ArrowType::UPRIGHT:
      tiles = spr_arrow_upleftTiles;
      size = spr_arrow_upleftTilesLen;
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      tiles = spr_arrow_downleftTiles;
      size = spr_arrow_downleftTilesLen;
      flip = true;
      break;
  }

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(tiles, size)
               .withSize(SIZE_16_16)
               .withAnimated(ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(GBA_SCREEN_WIDTH - 1, GBA_SCREEN_HEIGHT - 1)
               .buildPtr();

  if (id > 0) {
    // reuse previous arrow tiles
    sprite->setData(NULL);
    sprite->setImageSize(0);
  }

  this->type = type;
  this->flip = flip;
}

void Arrow::initialize() {
  sprite->moveTo(ARROW_CORNER_MARGIN + ARROW_MARGIN * type, GBA_SCREEN_HEIGHT);
}

void Arrow::discard() {
  sprite->moveTo(GBA_SCREEN_WIDTH - 1, GBA_SCREEN_HEIGHT - 1);
}

ArrowState Arrow::update() {
  sprite->flipHorizontally(flip);

  sprite->moveTo(sprite->getX(), sprite->getY() - 3);

  return sprite->getY() < ARROW_CORNER_MARGIN ? ArrowState::OUT
                                              : ArrowState::ACTIVE;
}

Sprite* Arrow::get() {
  return sprite.get();
}
