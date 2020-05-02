#include "Darkener.h"

#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

u32 BANK_TILES = 3;
u32 BANK_MAP = 28;
u32 COLOR_INDEX = 255;
u32 MAX_COL = 12;
u8 TRANSPARENT_TILE = 254;
u8 BLACK_TILE = 255;
u32 OPACITY = 10;

Darkener::Darkener(u8 id) {
  this->id = id;
}

void Darkener::initialize() {
  BACKGROUND_setup(id, BANK_TILES, BANK_MAP);

  BACKGROUND_setColor(COLOR_INDEX, 0);
  BACKGROUND_createSolidTile(BANK_TILES, TRANSPARENT_TILE, 0);
  BACKGROUND_createSolidTile(BANK_TILES, BLACK_TILE, COLOR_INDEX);
  BACKGROUND_fillMap(BANK_MAP, [](u8 row, u8 col) {
    return col < MAX_COL ? BLACK_TILE : TRANSPARENT_TILE;
  });

  EFFECT_setUpBlend(BLD_BG[id], BLD_BG[id + 1]);
  EFFECT_setBlendAlpha(OPACITY);
}
