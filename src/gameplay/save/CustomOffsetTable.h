#ifndef CUSTOM_OFFSET_TABLE_H
#define CUSTOM_OFFSET_TABLE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "ArcadeProgress.h"

#define CUSTOM_OFFSET_TABLE_SIZE (ARCADE_MAX_LEVELS * ARCADE_MAX_SONGS)
// 10000 registers: One per numeric difficulty (max=100) per song (max=100)
// => 1 register per byte => 10000 bytes total

#define CUSTOM_OFFSET_MAGIC_INDEX 99
#define CUSTOM_OFFSET_MAGIC_LEVEL 0
#define CUSTOM_OFFSET_MAGIC_VALUE (-83)

void OFFSET_initialize();
bool OFFSET_isInitialized();

u32 OFFSET_getCount();
s8 OFFSET_get(u8 songId, u8 levelIndex);
void OFFSET_set(u8 songId, u8 levelIndex, int offset);

#endif  // CUSTOM_OFFSET_TABLE_H
