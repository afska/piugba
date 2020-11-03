#include "ArcadeProgress.h"

#include "SaveFile.h"

// TODO: Test and use

#define ARCADE_read(ARRAY, SONG_INDEX, LEVEL)               \
  u32 index = (SONG_INDEX * ARCADE_MAX_LEVELS + LEVEL) / 2; \
  u32 part = (SONG_INDEX * ARCADE_MAX_LEVELS + LEVEL) % 2;  \
  u8 n = SAVEFILE_read8(ARRAY[index]);                      \
  u8 value = part == 0 ? n & 0b111 : (n & 0b111000) >> 3;   \
  if (value == 0)                                           \
    return GradeType::UNPLAYED;                             \
                                                            \
  return static_cast<GradeType>(value - 1);

#define ARCADE_write(ARRAY, SONG_INDEX, LEVEL, GRADE)                    \
  u32 index = (SONG_INDEX * ARCADE_MAX_LEVELS + LEVEL) / 2;              \
  u32 part = (SONG_INDEX * ARCADE_MAX_LEVELS + LEVEL) % 2;               \
  u8 n = SAVEFILE_read8(ARRAY[index]);                                   \
  u8 value = grade + 1;                                                  \
  u8 updatedN =                                                          \
      part == 0 ? (n & ~0b111) | value : (n & ~0b111000) | (value << 3); \
  SAVEFILE_write8(ARRAY[index], updatedN);

void ARCADE_initialize() {
  for (u32 i = 0; i < ARCADE_PROGRESS_SIZE; i++) {
    SAVEFILE_write8(SRAM->singleArcadeProgress[i], 0);
    SAVEFILE_write8(SRAM->doubleArcadeProgress[i], 0);
  }
}

GradeType ARCADE_readSingle(u8 songIndex, u8 level) {
  ARCADE_read(SRAM->singleArcadeProgress, songIndex, level);
}

GradeType ARCADE_readDouble(u8 songIndex, u8 level) {
  ARCADE_read(SRAM->doubleArcadeProgress, songIndex, level);
}

void ARCADE_writeSingle(u8 songIndex, u8 level, GradeType grade) {
  ARCADE_write(SRAM->singleArcadeProgress, songIndex, level, grade);
}

void ARCADE_writeDouble(u8 songIndex, u8 level, GradeType grade) {
  ARCADE_write(SRAM->doubleArcadeProgress, songIndex, level, grade);
}
