#include "Darkener.h"

#include "gameplay/save/SaveFile.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

const u32 BGA_DARK_HALF_START_COL = 0;
const u32 BGA_DARK_HALF_END_COL = 12;
const u32 BGA_DARK_FULL_END_COL = 30;

const u32 OPACITY = 10;
const u32 BANK_TILES = 3;
const u32 BANK_MAP = 28;
const u8 COLOR_INDEX = 255;
const u8 TRANSPARENT_TILE = 254;
const u8 BLACK_TILE = 255;

Darkener::Darkener(u8 id, u8 priority) {
  this->id = id;
  this->priority = priority;
}

void Darkener::initialize(BackgroundType type) {
  initialize(type, COLOR_INDEX);
  this->x = 0;
}

void Darkener::initialize(BackgroundType type, u8 colorIndex) {
  BACKGROUND_setup(id, BANK_TILES, BANK_MAP, priority);

  BACKGROUND_setColor(colorIndex, 0);
  BACKGROUND_createSolidTile(BANK_TILES, TRANSPARENT_TILE, 0);
  BACKGROUND_createSolidTile(BANK_TILES, BLACK_TILE, colorIndex);
  BACKGROUND_fillMap(BANK_MAP, [&type](u8 row, u8 col) {
    switch (type) {
      case BackgroundType::RAW:
        return TRANSPARENT_TILE;
      case BackgroundType::HALF_BGA_DARK:
        return col >= BGA_DARK_HALF_START_COL && col < BGA_DARK_HALF_END_COL
                   ? BLACK_TILE
                   : TRANSPARENT_TILE;
      case BackgroundType::FULL_BGA_DARK:
      default:
        return col < BGA_DARK_FULL_END_COL ? BLACK_TILE : TRANSPARENT_TILE;
    }
  });

  EFFECT_setUpBlend(BLD_BG[id], BLD_BG[id + 1]);
  EFFECT_setBlendAlpha(OPACITY);
}
