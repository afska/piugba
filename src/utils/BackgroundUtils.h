#ifndef BACKGROUND_UTILS_H
#define BACKGROUND_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

const u32 TILE_SIZE = 16;

inline void BACKGROUND_enable(bool bg0, bool bg1, bool bg2, bool bg3) {
  REG_DISPCNT = bg0 ? REG_DISPCNT | DCNT_BG0 : REG_DISPCNT & ~DCNT_BG0;
  REG_DISPCNT = bg1 ? REG_DISPCNT | DCNT_BG1 : REG_DISPCNT & ~DCNT_BG1;
  REG_DISPCNT = bg2 ? REG_DISPCNT | DCNT_BG2 : REG_DISPCNT & ~DCNT_BG2;
  REG_DISPCNT = bg3 ? REG_DISPCNT | DCNT_BG3 : REG_DISPCNT & ~DCNT_BG3;
}

inline void BACKGROUND_setup(u8 id, u8 charblock, u8 screenblock) {
  REG_BGCNT[id] =
      BG_CBB(charblock) | BG_SBB(screenblock) | BG_8BPP | BG_REG_32x32 | id;
}

inline void BACKGROUND_loadPalette(const unsigned int data[],
                                   u32 length,
                                   u32 start) {
  for (u32 colorIndex = 0; colorIndex < length; colorIndex++)
    pal_bg_mem[start + colorIndex] = data[colorIndex];
}

inline void BACKGROUND_loadTiles(const unsigned int data[],
                                 u32 length,
                                 u8 charblock,
                                 u32 start) {
  u32 tileIndex = 0;
  int part = -1;
  for (u32 i = 0; i < length; i++) {
    part++;
    if (part == TILE_SIZE) {
      tileIndex++;
      part = 0;
    }

    tile8_mem[charblock][start + tileIndex].data[part] = data[i];
  }
}

inline void BACKGROUND_loadMap(const unsigned int data[],
                               u32 length,
                               u8 screenblock,
                               u32 start,
                               u32 end,
                               u8 transparentColor) {
  for (u32 entryIndex = 0; entryIndex < length; entryIndex++)
    se_mem[screenblock][entryIndex] = entryIndex >= start && entryIndex < end
                                          ? data[entryIndex - start]
                                          : transparentColor;
}

inline void BACKGROUND_setColor(u8 index, u8 value) {
  pal_bg_mem[index] = value;
}

inline void BACKGROUND_createSolidTile(u8 charblock, u8 tile, u8 colorIndex) {
  for (u8 line = 0; line < 8; line++) {
    tile8_mem[charblock][tile].data[line * 2] =
        (colorIndex << 0) + (colorIndex << 8) + (colorIndex << 16) +
        (colorIndex << 24);
    tile8_mem[charblock][tile].data[line * 2 + 1] =
        (colorIndex << 0) + (colorIndex << 8) + (colorIndex << 16) +
        (colorIndex << 24);
  }
}

template <typename T>
inline void BACKGROUND_fillMap(u8 screenblock, T getTile) {
  for (u32 i = 0; i < 1024; i++) {
    u8 row = i / 32;
    u8 col = i % 32;
    se_mem[screenblock][i] = getTile(row, col);
  }
}

#endif  // BACKGROUND_UTILS_H