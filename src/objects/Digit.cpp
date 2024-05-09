#include "Digit.h"

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_numbers.h"
#include "data/content/_compiled_sprites/spr_numbers_mdrn.h"
#include "data/content/_compiled_sprites/spr_numbers_mini.h"
#include "data/content/_compiled_sprites/spr_numbers_mini_mdrn.h"
#include "gameplay/save/SaveFile.h"
#include "utils/SpriteUtils.h"

const u32 DIGIT_WIDTHS[] = {26, 19};
const u32 RED_OFFSET = 10;

Digit::Digit(DigitSize size, u32 x, u32 y, u32 index, bool reuseTiles) {
  relocate(size, x, y, index);
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(
              size == DigitSize::BIG
                  ? (SAVEFILE_isUsingModernTheme() ? spr_numbers_mdrnTiles
                                                   : spr_numbersTiles)
                  : (SAVEFILE_isUsingModernTheme() ? spr_numbers_mini_mdrnTiles
                                                   : spr_numbers_miniTiles),
              size == DigitSize::BIG ? (SAVEFILE_isUsingModernTheme()
                                            ? sizeof(spr_numbers_mdrnTiles)
                                            : sizeof(spr_numbersTiles))
                                     : (SAVEFILE_isUsingModernTheme()
                                            ? sizeof(spr_numbers_mini_mdrnTiles)
                                            : sizeof(spr_numbers_miniTiles)))
          .withSize(SIZE_32_16)
          .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
          .buildPtr();

  if (reuseTiles || index > 0)
    SPRITE_reuseTiles(sprite.get());
}

void Digit::set(u32 value, bool isRed) {
  SPRITE_goToFrame(sprite.get(), value + (isRed ? RED_OFFSET : 0));
}

void Digit::relocate(DigitSize size, u32 x, u32 y, u32 index) {
  animationPositionX = x + index * DIGIT_WIDTHS[size];
  animationPositionY = y;
}

Sprite* Digit::get() {
  return sprite.get();
}
