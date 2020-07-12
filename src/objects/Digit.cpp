#include "Digit.h"

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_numbers.h"
#include "data/content/_compiled_sprites/spr_numbers_mini.h"
#include "utils/SpriteUtils.h"

const u32 DIGIT_WIDTHS[] = {26, 19};
const u32 RED_OFFSET = 10;

Digit::Digit(DigitSize size, u32 x, u32 y, u32 index) {
  animationPositionX = x + index * DIGIT_WIDTHS[size];
  animationPositionY = y;
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite = builder
               .withData(size == DigitSize::BIG ? spr_numbersTiles
                                                : spr_numbers_miniTiles,
                         size == DigitSize::BIG ? sizeof(spr_numbersTiles)
                                                : sizeof(spr_numbers_miniTiles))
               .withSize(SIZE_32_16)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();

  if (index > 0)
    SPRITE_reuseTiles(sprite.get());
}

void Digit::set(u32 value, bool isRed) {
  SPRITE_goToFrame(sprite.get(), value + (isRed ? RED_OFFSET : 0));
}

Sprite* Digit::get() {
  return sprite.get();
}
