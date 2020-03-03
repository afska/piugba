#include "ComboDigit.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_number0.h"
#include "data/content/compiled/spr_number1.h"

ComboDigit::ComboDigit(u32 value, u32 position) {
  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(value == 0 ? spr_number0Tiles : spr_number1Tiles,
                    sizeof(value == 0 ? spr_number0Tiles : spr_number1Tiles))
          .withSize(SIZE_32_16)
          .withLocation(7 + position * 24, 100)
          .buildPtr();
}

Sprite* ComboDigit::get() {
  return sprite.get();
}
