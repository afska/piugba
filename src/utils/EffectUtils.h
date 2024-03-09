#ifndef EFFECT_UTILS_H
#define EFFECT_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

const u32 MIN_OPACITY = 0;
const u32 MAX_OPACITY = 16;
const u32 MIN_MOSAIC = 0;
const u32 MAX_MOSAIC = 15;
const u8 BLD_BG[] = {BLD_BG0, BLD_BG1, BLD_BG2, BLD_BG3};
const u8 BLD_MODE_OFF = 0;
const u8 BLD_MODE_NORMAL = 1;

extern u16 regBldCnt, regBldAlpha, regMosaic;

inline void EFFECT_render() {
  REG_BLDCNT = regBldCnt;
  REG_BLDALPHA = regBldAlpha;
  REG_MOSAIC = regMosaic;
}

// Top and bottom values: BLD_BG[0~3], BLD_OBJ, BLD_ALL
inline void EFFECT_setUpBlend(u8 top, u8 bottom) {
  regBldCnt = BLD_BUILD(top, bottom, BLD_MODE_NORMAL);
}

// Opacity is in [MIN_OPACITY, MAX_OPACITY]
inline void EFFECT_setBlendAlpha(u8 topOpacity) {
  regBldAlpha = BLDA_BUILD(topOpacity, MAX_OPACITY - topOpacity);
}

inline void EFFECT_turnOffBlend() {
  regBldCnt = BLD_BUILD(0, 0, BLD_MODE_OFF);
}

// Value is in [0, MAX_MOSAIC]
inline void EFFECT_setMosaic(u8 value) {
  regMosaic = MOS_BUILD(value, value, value, value);
}

inline void EFFECT_turnOffMosaic() {
  regMosaic = MOS_BUILD(0, 0, 0, 0);
}

#endif  // EFFECT_UTILS_H
