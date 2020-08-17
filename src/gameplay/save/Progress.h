#ifndef PROGRESS_H
#define PROGRESS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"

#define MAX_PROGRESS_REGISTERS 100

typedef struct __attribute__((__packed__)) {
  u8 isArcadeModeUnlocked;
  u8 isImpossibleModeUnlocked;
} GlobalProgress;

typedef struct __attribute__((__packed__)) {
  u32 completedSongs;
  u8 grades[MAX_PROGRESS_REGISTERS];
} Progress;

#endif  // PROGRESS_H
