#ifndef ARCADE_PROGRESS_H
#define ARCADE_PROGRESS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"

#define ARCADE_MAX_LEVELS 100
#define ARCADE_MAX_SONGS 100
#define ARCADE_PROGRESS_SIZE (ARCADE_MAX_LEVELS * ARCADE_MAX_SONGS / 2)
// 10000 registers: One per numeric difficulty (max=100) per song (max=100)
// => 2 registers per byte (3 bit each) => 5000 bytes total

void ARCADE_initialize();

GradeType ARCADE_readSingle(u8 songId, u8 level);
GradeType ARCADE_readDouble(u8 songId, u8 level);
void ARCADE_writeSingle(u8 songId, u8 level, GradeType grade);
void ARCADE_writeDouble(u8 songId, u8 level, GradeType grade);

#endif  // ARCADE_PROGRESS_H
