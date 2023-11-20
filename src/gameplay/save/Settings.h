#ifndef SETTINGS_H
#define SETTINGS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum GamePosition : u8 { LEFT, MIDDLE, RIGHT };
enum BackgroundType : u8 { RAW, HALF_BGA_DARK, FULL_BGA_DARK };
enum BGADarkBlink : u8 { BLINK_OFF, BLINK_SLOW, BLINK_FAST };

typedef struct __attribute__((__packed__)) {
  u32 audioLag;
  GamePosition gamePosition;
  BackgroundType backgroundType;
  BGADarkBlink bgaDarkBlink;
} Settings;

#endif  // SETTINGS_H
