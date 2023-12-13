#include "CustomOffsetTable.h"

#include "SaveFile.h"

#define LIMIT 120

void OFFSET_initialize() {
  for (u32 i = 0; i < CUSTOM_OFFSET_TABLE_SIZE; i++) {
    SAVEFILE_write8(SRAM->customOffsets[i], 0);
  }

  OFFSET_set(CUSTOM_OFFSET_MAGIC_INDEX, CUSTOM_OFFSET_MAGIC_LEVEL,
             CUSTOM_OFFSET_MAGIC_VALUE);
}

bool OFFSET_isInitialized() {
  return OFFSET_get(CUSTOM_OFFSET_MAGIC_INDEX, CUSTOM_OFFSET_MAGIC_LEVEL) ==
         CUSTOM_OFFSET_MAGIC_VALUE;
}

u32 OFFSET_getCount() {
  u32 count = 0;
  for (u32 i = 0; i < CUSTOM_OFFSET_TABLE_SIZE - ARCADE_MAX_LEVELS; i++) {
    // (`-ARCADE_MAX_LEVELS` to exclude magic register)
    s8 offset = SAVEFILE_read8(SRAM->customOffsets[i]);
    if (offset != 0)
      count++;
  }
  return count;
}

s8 OFFSET_get(u8 songId, u8 levelIndex) {
  s8 offset = (s8)SAVEFILE_read8(
      SRAM->customOffsets[songId * ARCADE_MAX_LEVELS + levelIndex]);
  return max(min(offset, LIMIT), -LIMIT);
}

void OFFSET_set(u8 songId, u8 levelIndex, int offset) {
  SAVEFILE_write8(SRAM->customOffsets[songId * ARCADE_MAX_LEVELS + levelIndex],
                  max(min(offset, LIMIT), -LIMIT));
}
