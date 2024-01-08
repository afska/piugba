#ifndef PROGRESS_H
#define PROGRESS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"

#define MAX_PROGRESS_REGISTERS 100

typedef struct __attribute__((__packed__)) {
  u8 completedSongs;
  u8 grades[MAX_PROGRESS_REGISTERS];
} Progress;

typedef struct __attribute__((__packed__)) {
  u8 completedSongs[3];
  u8 grades[3];
} DeathMixProgress;

#endif  // PROGRESS_H
