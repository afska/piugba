#include "ComboDigit.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_numbers.h"
#include "utils/SpriteUtils.h";

ComboDigit::ComboDigit(u32 position) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_numbersTiles, sizeof(spr_numbersTiles))
               .withSize(SIZE_32_16)
               .withLocation(8 + position * 26, 89)
               .buildPtr();

  if (position > 0)
    SpriteUtils::reuseTiles(sprite.get());
}

void ComboDigit::set(u32 value) {
  SpriteUtils::goToFrame(sprite.get(), value);
}

Sprite* ComboDigit::get() {
  return sprite.get();
}
