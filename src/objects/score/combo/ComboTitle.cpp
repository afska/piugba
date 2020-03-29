#include "ComboTitle.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_combo.h"

const u32 POSITION_X = 16;
const u32 POSITION_Y = 70;

ComboTitle::ComboTitle() {
  animationPositionX = POSITION_X;
  animationPositionY = POSITION_Y;
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_comboTiles, sizeof(spr_comboTiles))
               .withSize(SIZE_64_32)
               .withLocation(POSITION_X, POSITION_Y)
               .buildPtr();
}

Sprite* ComboTitle::get() {
  return sprite.get();
}
