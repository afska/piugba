#ifndef ARCADE_PROGRESS_H
#define ARCADE_PROGRESS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"

#define ARCADE_MAX_LEVELS 100
#define ARCADE_MAX_SONGS 100
#define ARCADE_PROGRESS_SIZE (ARCADE_MAX_LEVELS * ARCADE_MAX_SONGS / 2)
// 10000 registers: One per numeric difficulty (max=100) per song (max=100)
// => 2 registers per byte (3 bit each) => 5000 bytes total
// The grades are stored by index. Before v1.8.0, they were stored by level.

void ARCADE_initialize();
bool ARCADE_isInitialized();
bool ARCADE_isLegacy();

GradeType ARCADE_readSingle(u8 songId, u8 levelIndex);
GradeType ARCADE_readDouble(u8 songId, u8 levelIndex);
void ARCADE_writeSingle(u8 songId, u8 levelIndex, GradeType grade);
void ARCADE_writeDouble(u8 songId, u8 levelIndex, GradeType grade);

void ARCADE_migrate();

#endif  // ARCADE_PROGRESS_H
