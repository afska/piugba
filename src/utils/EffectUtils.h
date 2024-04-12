#ifndef EFFECT_UTILS_H
#define EFFECT_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#define HIDE_SPRITE (1 << 9)
#define MAX_AFFINES 4
#define AFFINE_BASE (32 - MAX_AFFINES)

const u32 MIN_OPACITY = 0;
const u32 MAX_OPACITY = 16;
const u32 MIN_MOSAIC = 0;
const u32 MAX_MOSAIC = 15;
const u8 BLD_BG[] = {BLD_BG0, BLD_BG1, BLD_BG2, BLD_BG3};
const u8 BLD_MODE_OFF = 0;
const u8 BLD_MODE_NORMAL = 1;

const u32 BREATH_SCALE_LUT[] = {256, 244, 233, 223, 213, 205, 209, 213,
                                218, 223, 228, 233, 238, 244, 250};
const u32 BREATH_STEPS = 15;

extern u16 regBldCnt, regBldAlpha, regMosaic;
extern OBJ_AFFINE affine[MAX_AFFINES];

inline void EFFECT_render() {
  REG_BLDCNT = regBldCnt;
  REG_BLDALPHA = regBldAlpha;
  REG_MOSAIC = regMosaic;
  for (u32 i = 0; i < MAX_AFFINES; i++)
    obj_aff_mem[AFFINE_BASE + i] = affine[i];
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

// AffineID is in [0, 5], scaleX/scaleY are 8.8 fixed-point numbers
inline void EFFECT_setScale(u32 affineId, u32 scaleX, u32 scaleY) {
  affine[affineId].pa = scaleX;
  affine[affineId].pb = 0;
  affine[affineId].pc = 0;
  affine[affineId].pd = scaleY;
}

inline void EFFECT_clearAffine() {
  for (u32 i = 0; i < MAX_AFFINES; i++) {
    affine[i].fill0[0] = HIDE_SPRITE;
    affine[i].fill1[0] = HIDE_SPRITE;
    affine[i].fill2[0] = HIDE_SPRITE;
    affine[i].fill3[0] = HIDE_SPRITE;
    affine[i].pa = 0x100;
    affine[i].pb = 0;
    affine[i].pc = 0;
    affine[i].pd = 0x100;
  }
}

#endif  // EFFECT_UTILS_H
