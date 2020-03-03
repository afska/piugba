#include "Combo.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/spr_combo.h"

Combo::Combo() {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_comboTiles, sizeof(spr_comboTiles))
               .withSize(SIZE_64_32)
               .withLocation(16, 80)
               .buildPtr();
}

Sprite* Combo::get() {
  return sprite.get();
}
