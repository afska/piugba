#ifndef MEMORY_H
#define MEMORY_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct __attribute__((__packed__)) {
  u8 pageIndex;
  u8 songIndex;
  u8 difficultyLevel;
  u8 numericLevel;
  bool isAudioLagCalibrated;
} Memory;

#endif  // MEMORY_H
