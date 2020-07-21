#ifndef SETTINGS_H
#define SETTINGS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum BackgroundType { RAW, HALF_BGA_DARK, FULL_BGA_DARK };

typedef struct {
  int audioLag;
  bool pixelize;
  u8 holderPosition;  // (0 = left, 1 = middle, 2 = right)
  BackgroundType backgroundType;
  bool bgaDarkBlink;
} Settings;

#endif  // SETTINGS_H
