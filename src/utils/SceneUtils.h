#ifndef SCENE_UTILS_H
#define SCENE_UTILS_H

#include <string>

#include "BackgroundUtils.h"
#include "EffectUtils.h"
#include "SpriteUtils.h"

const u32 TEXT_MIDDLE_COL = 12;

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

inline void SCENE_invert(PaletteManager* palette) {
  for (int bank = 0; bank < PALETTE_BANK_SIZE; bank++) {
    for (int index = 0; index < PALETTE_BANK_SIZE; index++) {
      auto color = palette->get(bank, index);
      palette->change(bank, index, 0xffff - color);
    }
  }
}

#endif  // SCENE_UTILS_H
