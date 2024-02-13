#ifndef SCENE_UTILS_H
#define SCENE_UTILS_H

#include <libgba-sprite-engine/background/text_stream.h>

#include <string>

#include "BackgroundUtils.h"
#include "EffectUtils.h"
#include "FadeOutPixelTransitionEffect.h"
#include "PixelTransitionEffect.h"
#include "SpriteUtils.h"
#include "utils/IOPort.h"
#include "utils/Rumble.h"

extern "C" {
#include "player/player.h"
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
  NIGHT,
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
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();
  TextStream::instance().clear();
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

inline void SCENE_softReset() {
  player_stop();
  player_unload();

  RUMBLE_stop();
  IOPORT_sdLow();
  RegisterRamReset(RESET_REG | RESET_VRAM);
  SoftReset();
}

#endif  // SCENE_UTILS_H
