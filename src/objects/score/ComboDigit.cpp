#include "ComboDigit.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_numbers.h"
#include "utils/SpriteUtils.h";

ComboDigit::ComboDigit(u32 position) {
  x = 8 + position * 26;
  y = 89;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_numbersTiles, sizeof(spr_numbersTiles))
               .withSize(SIZE_32_16)
               .withLocation(x, y)
               .buildPtr();

  if (position > 0)
    SpriteUtils::reuseTiles(sprite.get());
}

void ComboDigit::set(u32 value) {
  SpriteUtils::goToFrame(sprite.get(), value);
}

void ComboDigit::show() {
  sprite->moveTo(x, y);
}

void ComboDigit::hide() {
  SpriteUtils::hide(sprite.get());
}

Sprite* ComboDigit::get() {
  return sprite.get();
}
