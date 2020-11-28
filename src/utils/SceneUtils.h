#ifndef SCENE_UTILS_H
#define SCENE_UTILS_H

#include <libgba-sprite-engine/background/text_stream.h>

#include <string>

#include "BackgroundUtils.h"
#include "EffectUtils.h"
#include "SpriteUtils.h"

const u32 TEXT_MIDDLE_COL = 12;
const u32 TEXT_TOTAL_COLS = 30;

inline void SCENE_init() {
  TextStream::instance().clear();
  TextStream::instance().scroll(0, 0);
  TextStream::instance().setMosaic(false);

  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();
}

inline void SCENE_write(std::string text, u32 row) {
  TextStream::instance().setText(text, row,
                                 TEXT_MIDDLE_COL - text.length() / 2);
}

inline void SCENE_decolorize(PaletteManager* palette, u8 mode) {
  for (int bank = 0; bank < PALETTE_BANK_SIZE; bank++) {
    for (int index = 0; index < PALETTE_BANK_SIZE; index++) {
      auto color = palette->get(bank, index);

      u8 r = color & 0b11111;
      u8 g = (color & 0b1111100000) >> 5;
      u8 b = (color & 0b111110000000000) >> 10;

      if (mode == 1)
        color = 0xffff - color;
      else if (mode == 2)
        color = r | (r << 5) | (r << 10);
      else if (mode == 3)
        color = r;
      else if (mode == 4)
        color = g << 5;
      else if (mode == 5)
        color = b << 10;

      palette->change(bank, index, color);
    }
  }
}

#endif  // SCENE_UTILS_H
