#include "ArcadeProgress.h"

#include "SaveFile.h"

#define REGISTER_PART0 0b111
#define REGISTER_PART1 0b111000

#define ARCADE_read(ARRAY, SONG_ID, LEVEL)                               \
  u32 index = (SONG_ID * ARCADE_MAX_LEVELS + LEVEL) / 2;                 \
  u32 part = (SONG_ID * ARCADE_MAX_LEVELS + LEVEL) % 2;                  \
  u8 n = SAVEFILE_read8(ARRAY[index]);                                   \
  u8 value = part == 0 ? n & REGISTER_PART0 : (n & REGISTER_PART1) >> 3; \
  if (value == 0)                                                        \
    return GradeType::UNPLAYED;                                          \
                                                                         \
  return static_cast<GradeType>(value - 1);

#define ARCADE_write(ARRAY, SONG_ID, LEVEL, GRADE)                \
  u32 index = (SONG_ID * ARCADE_MAX_LEVELS + LEVEL) / 2;          \
  u32 part = (SONG_ID * ARCADE_MAX_LEVELS + LEVEL) % 2;           \
  u8 n = SAVEFILE_read8(ARRAY[index]);                            \
  u8 value = grade + 1;                                           \
  u8 updatedN = part == 0 ? (n & ~REGISTER_PART0) | value         \
                          : (n & ~REGISTER_PART1) | (value << 3); \
  SAVEFILE_write8(ARRAY[index], updatedN);

void ARCADE_initialize() {
  for (u32 i = 0; i < ARCADE_PROGRESS_SIZE; i++) {
    SAVEFILE_write8(SRAM->singleArcadeProgress[i], 0);
    SAVEFILE_write8(SRAM->doubleArcadeProgress[i], 0);
  }

  ARCADE_writeSingle(0, 0, GradeType::C);
}

bool ARCADE_isInitialized() {
  return ARCADE_readSingle(0, 0) == GradeType::C;
}

GradeType ARCADE_readSingle(u8 songId, u8 level) {
  ARCADE_read(SRAM->singleArcadeProgress, songId, level);
}

GradeType ARCADE_readDouble(u8 songId, u8 level) {
  ARCADE_read(SRAM->doubleArcadeProgress, songId, level);
}

void ARCADE_writeSingle(u8 songId, u8 level, GradeType grade) {
  ARCADE_write(SRAM->singleArcadeProgress, songId, level, grade);
}

void ARCADE_writeDouble(u8 songId, u8 level, GradeType grade) {
  ARCADE_write(SRAM->doubleArcadeProgress, songId, level, grade);
}
