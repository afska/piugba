#include "Arrow.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/arrow_center.h"
#include "data/arrow_downleft.h"
#include "data/arrow_upleft.h"

const int ANIMATION_FRAMES = 5;
const int ANIMATION_DELAY = 2;

Arrow::Arrow(u32 id, ArrowType type) {
  this->id = id;

  const unsigned int* tiles;
  int size;
  bool flip = false;
  switch (type) {
    case ArrowType::DOWNLEFT:
      tiles = arrow_downleftTiles;
      size = arrow_downleftTilesLen;
      break;
    case ArrowType::UPLEFT:
      tiles = arrow_upleftTiles;
      size = arrow_upleftTilesLen;
      break;
    case ArrowType::CENTER:
      tiles = arrow_centerTiles;
      size = arrow_centerTilesLen;
      break;
    case ArrowType::UPRIGHT:
      tiles = arrow_upleftTiles;
      size = arrow_upleftTilesLen;
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      tiles = arrow_downleftTiles;
      size = arrow_downleftTilesLen;
      flip = true;
      break;
  }

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(tiles, size)
               .withSize(SIZE_16_16)
               .withAnimated(ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(ARROW_CORNER_MARGIN + ARROW_MARGIN * type,
                             GBA_SCREEN_HEIGHT)
               .buildPtr();
  this->type = type;
  this->flip = flip;
}

void Arrow::initialize() {
  sprite->moveTo(ARROW_CORNER_MARGIN + ARROW_MARGIN * type, GBA_SCREEN_HEIGHT);
}

void Arrow::discard() {
  sprite->moveTo(GBA_SCREEN_WIDTH - 1, GBA_SCREEN_HEIGHT - 1);
}

u32 Arrow::getId() {
  return id;
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
