#ifndef SETTINGS_H
#define SETTINGS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum HolderPosition : u8 { LEFT, MIDDLE, RIGHT };
enum BackgroundType : u8 { RAW, HALF_BGA_DARK, FULL_BGA_DARK };

typedef struct {
  u32 audioLag;
  u8 pixelize;
  u8 holderPosition;
  u8 backgroundType;
  u8 bgaDarkBlink;
} Settings;

#endif  // SETTINGS_H
