#include "Darkener.h"

#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

u32 BANK_TILES = 2;
u32 BANK_MAP = 22;
u32 COLOR_INDEX = 255;
u32 MAX_COL = 12;
u32 OPACITY = 10;

Darkener::Darkener(u8 id) {
  this->id = id;
}

void Darkener::initialize() {
  BACKGROUND_setup(id, BANK_TILES, BANK_MAP);

  BACKGROUND_setColor(COLOR_INDEX, 0);
  BACKGROUND_createSolidTile(BANK_TILES, 0, 0);
  BACKGROUND_createSolidTile(BANK_TILES, 1, COLOR_INDEX);
  BACKGROUND_fillMap(BANK_MAP,
                     [](u8 row, u8 col) { return col < MAX_COL ? 1 : 0; });

  EFFECT_setUpBlend(BLD_BG[id], BLD_BG[id + 1]);
  EFFECT_setBlendAlpha(OPACITY);
}
