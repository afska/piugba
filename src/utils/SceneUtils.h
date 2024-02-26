#ifndef SCENE_UTILS_H
#define SCENE_UTILS_H

#include <libgba-sprite-engine/background/text_stream.h>

#include <string>

#include "BackgroundUtils.h"
#include "EffectUtils.h"
#include "PixelTransitionEffect.h"
#include "SpriteUtils.h"
#include "utils/IOPort.h"
#include "utils/Rumble.h"

extern "C" {
#include "player/player.h"
#include "utils/flashcartio/flashcartio.h"
}

const u32 TEXT_MIDDLE_COL = 12;
const u32 TEXT_TOTAL_COLS = 30;

enum ColorFilter {
  NO_FILTER,
  VIBRANT,
  CONTRAST,
  POSTERIZE,
  WARM,
  COLD,
  ETHEREAL,
  WATER,
  GOLDEN,
  DREAMY,
  RETRO,
  ALIEN,
  SPACE,
  SEPIA,
  GRAYSCALE,
  MONO,
  INVERT
};

inline void SCENE_init() {
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  EFFECT_render();
  TextStream::instance().scrollNow(0, 0);
  TextStream::instance().setMosaic(false);
}

inline void SCENE_write(std::string text, u32 row) {
  TextStream::instance().setText(text, row,
                                 TEXT_MIDDLE_COL - text.length() / 2);
}

COLOR SCENE_transformColor(COLOR color, ColorFilter filter);

void SCENE_applyColorFilterIndex(PaletteManager* palette,
                                 int bank,
                                 int index,
                                 ColorFilter filter);

void SCENE_applyColorFilter(PaletteManager* palette, ColorFilter colorFilter);

inline void SCENE_wait(u32 verticalLines) {
  u32 lines = 0;
  u32 vCount = REG_VCOUNT;

  while (lines < verticalLines) {
    if (REG_VCOUNT != vCount) {
      lines++;
      vCount = REG_VCOUNT;
    }
  };
}

__attribute__((section(".ewram"))) extern u32 temp;
inline void SCENE_overclockEWRAM() {
  // tries to overclock EWRAM
  // but rollbacks if a GB Micro is detected to prevent crashes

  *((u32*)0x4000800) = (0x0E << 24) | (1 << 5);

  for (int index = 8; index >= 0; --index) {
    vu32* volatileTemp = (vu32*)&temp;
    u32 testValue = qran();
    *volatileTemp = testValue;

    if (*volatileTemp != testValue) {
      *((u32*)0x4000800) = (0x0D << 24) | (1 << 5);
      return;
    }
  }
}

inline void SCENE_softReset() {
  if (IS_FLASHCART_UNLOCKED)
    flashcartio_lock();

  VBlankIntrWait();
  REG_IME = 0;
  player_stop();
  player_unload();

  RUMBLE_stop();
  IOPORT_low();
  RegisterRamReset(RESET_VRAM | RESET_PALETTE | RESET_OAM | RESET_REG_SOUND |
                   RESET_REG);
  SoftReset();
}

#endif  // SCENE_UTILS_H
