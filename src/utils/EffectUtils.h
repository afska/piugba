#ifndef EFFECT_UTILS_H
#define EFFECT_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

const u32 MIN_BLEND = 0;
const u32 MAX_BLEND = 16;
const u8 BLD_BG[] = {BLD_BG0, BLD_BG1, BLD_BG2, BLD_BG3};
const u8 BLD_MODE_OFF = 0;
const u8 BLD_MODE_NORMAL = 1;

// Top and bottom values: BLD_BG[0~3], BLD_OBJ, BLD_ALL
inline void EFFECT_setUpBlend(u8 top, u8 bottom) {
  REG_BLDCNT = BLD_BUILD(top, bottom, BLD_MODE_NORMAL);
}

// Opacity is in [MIN_BLEND, MAX_BLEND]
inline void EFFECT_setBlendAlpha(u8 topOpacity) {
  REG_BLDALPHA = BLDA_BUILD(topOpacity, MAX_BLEND - topOpacity);
}

inline void EFFECT_setMosaic(u8 value) {
  REG_MOSAIC = MOS_BUILD(value, value, value, value);
}

#endif  // EFFECT_UTILS_H
