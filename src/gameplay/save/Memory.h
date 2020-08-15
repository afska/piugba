#ifndef MEMORY_H
#define MEMORY_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct {
  u8 pageIndex;
  u8 songIndex;
  u8 difficultyLevel;
  u8 multiplier;

  u8 isAudioLagCalibrated;
} Memory;

#endif  // MEMORY_H
