#ifndef SETTINGS_H
#define SETTINGS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum GamePosition : u8 { LEFT, MIDDLE, RIGHT };
enum BackgroundType : u8 { RAW, HALF_BGA_DARK, FULL_BGA_DARK };
enum Theme : u8 { CLASSIC, MODERN };

typedef struct __attribute__((__packed__)) {
  u32 audioLag;
  GamePosition gamePosition;
  BackgroundType backgroundType;
  bool bgaDarkBlink;
  Theme theme;
} Settings;

#endif  // SETTINGS_H
