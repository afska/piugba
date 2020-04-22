#include "ComboDigit.h"

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_numbers.h"
#include "utils/SpriteUtils.h"

const u32 DIGIT_POSITION_X = 8;
const u32 DIGIT_POSITION_Y = 89;
const u32 RED_OFFSET = 10;

ComboDigit::ComboDigit(u32 position) {
  animationPositionX = DIGIT_POSITION_X + position * DIGIT_WIDTH;
  animationPositionY = DIGIT_POSITION_Y;
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_numbersTiles, sizeof(spr_numbersTiles))
               .withSize(SIZE_32_16)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();

  if (position > 0)
    SPRITE_reuseTiles(sprite.get());
}

void ComboDigit::set(u32 value, bool isRed) {
  SPRITE_goToFrame(sprite.get(), value + (isRed ? RED_OFFSET : 0));
}

Sprite* ComboDigit::get() {
  return sprite.get();
}
