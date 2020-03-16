#include "ComboTitle.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_combo.h"

ComboTitle::ComboTitle() {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_comboTiles, sizeof(spr_comboTiles))
               .withSize(SIZE_64_32)
               .withLocation(16, 70)
               .buildPtr();
}

Sprite* ComboTitle::get() {
  return sprite.get();
}
