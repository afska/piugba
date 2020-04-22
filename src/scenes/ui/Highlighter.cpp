#include "Highlighter.h"

#include "data/custom/bg_selectionmask.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

Highlighter::Highlighter(u8 id) {
  this->id = id;
}

void Highlighter::initialize() {
  BACKGROUND_setup(id, BG_SELECTIONMASK_METADATA.TILES_BANK,
                   BG_SELECTIONMASK_METADATA.MAP_BANK);
  loadPalette();
  loadTiles();
  loadMap();

  EFFECT_setUpBlend(BLD_BG[id], BLD_BG[id + 1]);
  EFFECT_setBlendAlpha(13);
}

void Highlighter::select(u8 option) {
  selectedItem = option;
  loadTiles();
}

u8 Highlighter::getSelectedItem() {
  return selectedItem;
}

void Highlighter::loadPalette() {
  BACKGROUND_loadPalette(BG_SELECTIONMASK_PALETTE,
                         BG_SELECTIONMASK_METADATA.PALETTE_LENGTH,
                         BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX);

  pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX] = 0;
  pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 1] = 0;
  pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 2] = 0;
  pal_bg_mem[BG_SELECTIONMASK_METADATA.PALETTE_START_INDEX + 3] = 0;
}

void Highlighter::loadTiles() {
  BACKGROUND_loadTiles(BG_SELECTIONMASK_TILES[selectedItem],
                       BG_SELECTIONMASK_METADATA.TILES_LENGTH,
                       BG_SELECTIONMASK_METADATA.TILES_BANK,
                       BG_SELECTIONMASK_METADATA.TILES_START_INDEX);
}

void Highlighter::loadMap() {
  BACKGROUND_loadMap(BG_SELECTIONMASK_MAP,
                     BG_SELECTIONMASK_METADATA.MAP_TOTAL_TILES,
                     BG_SELECTIONMASK_METADATA.MAP_BANK,
                     BG_SELECTIONMASK_METADATA.MAP_START_INDEX,
                     BG_SELECTIONMASK_METADATA.MAP_END_INDEX,
                     BG_SELECTIONMASK_METADATA.TILES_START_INDEX);
}
