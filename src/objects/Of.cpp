#include "Of.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_of.h"

Of::Of(u32 x, u32 y) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_ofTiles, sizeof(spr_ofTiles))
               .withSize(SIZE_64_32)
               .withLocation(x, y)
               .buildPtr();
}

Sprite* Of::get() {
  return sprite.get();
}
