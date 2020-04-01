#include "LifeBar.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_lifebar.h"

const u32 POSITION_X = 15;
const int POSITION_Y = -11 + 2;
const u16 PALETTE_COLORS[] = {127, 4345, 410, 7606, 2686, 1595, 766, 700,  927,
                              894, 988,  923, 1017, 951,  974,  879, 9199, 936};
const u8 PALETTE_INDICES[] = {173, 175, 179, 180, 188, 186, 194, 190, 202,
                              197, 203, 199, 204, 198, 196, 193, 201, 192};
const COLOR CURSOR_COLOR = 0x7FD8;
const COLOR CURSOR_COLOR_BORDER = 0x7734;
const COLOR DISABLED_COLOR = 0x0000;
const COLOR DISABLED_COLOR_BORDER = 0x2529;

LifeBar::LifeBar() {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_lifebarTiles, sizeof(spr_lifebarTiles))
               .withSize(SIZE_64_32)
               .withLocation(POSITION_X, POSITION_Y)
               .buildPtr();
}

void LifeBar::tick(ForegroundPaletteManager* foregroundPalette) {
  paint(6, foregroundPalette);
}

void LifeBar::paint(u32 value, ForegroundPaletteManager* foregroundPalette) {
  bool odd = false;
  for (u32 i = 0; i < sizeof(PALETTE_INDICES); i++) {
    COLOR color = PALETTE_COLORS[i + odd];

    if (i >= value * 2)
      color = (odd ? DISABLED_COLOR_BORDER : DISABLED_COLOR);

    if (i >= value * 2 - 2 && i < value * 2)
      color = (odd ? CURSOR_COLOR_BORDER : CURSOR_COLOR);

    foregroundPalette->change(0, PALETTE_INDICES[i], color);
    odd = !odd;
  }
}

Sprite* LifeBar::get() {
  return sprite.get();
}
