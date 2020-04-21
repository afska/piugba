#ifndef EFFECT_UTILS_H
#define EFFECT_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

const u32 BLEND_LIMIT = 32;

// --- Combine with | ---
// BLD_BG0, BLD_BG1, BLD_BG2, BLD_BG3, BLD_OBJ, BLD_ALL
inline void EFFECT_setUpBlend(u8 top, u8 bottom) {
  REG_BLDCNT = BLD_BUILD(top, bottom, 1);
}

// Opacity is in [0, 32]
inline void EFFECT_setBlendAlpha(u8 topOpacity) {
  REG_BLDALPHA = BLDA_BUILD(topOpacity, BLEND_LIMIT - topOpacity);
}

#endif  // EFFECT_UTILS_H
