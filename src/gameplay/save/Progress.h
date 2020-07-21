#ifndef PROGRESS_H
#define PROGRESS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"

#define MAX_PROGRESS_REGISTERS 1000

typedef struct {
  u32 completedSongs;
  GradeType grades[MAX_PROGRESS_REGISTERS];
} Progress;

#endif  // PROGRESS_H
