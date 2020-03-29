#include "LifeBar.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_lifebar.h"

const u32 POSITION_X = 15;
const int POSITION_Y = -11 + 2;

LifeBar::LifeBar() {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_lifebarTiles, sizeof(spr_lifebarTiles))
               .withSize(SIZE_64_32)
               .withLocation(POSITION_X, POSITION_Y)
               .buildPtr();
}

Sprite* LifeBar::get() {
  return sprite.get();
}
