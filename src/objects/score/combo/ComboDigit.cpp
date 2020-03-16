#include "ComboDigit.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_numbers.h"
#include "utils/SpriteUtils.h";

const u32 POSITION_X = 8;
const u32 NUMBER_WIDTH = 26;
const u32 POSITION_Y = 89;

ComboDigit::ComboDigit(u32 position) {
  animationPositionX = POSITION_X + position * NUMBER_WIDTH;
  animationPositionY = POSITION_Y;
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_numbersTiles, sizeof(spr_numbersTiles))
               .withSize(SIZE_32_16)
               .withLocation(animationPositionX, animationPositionY)
               .buildPtr();

  if (position > 0)
    SPRITE_reuseTiles(sprite.get());
}

void ComboDigit::set(u32 value) {
  SPRITE_goToFrame(sprite.get(), value);
}

Sprite* ComboDigit::get() {
  return sprite.get();
}
