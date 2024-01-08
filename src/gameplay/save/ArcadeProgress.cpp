#include "ArcadeProgress.h"
#include "SaveFile.h"
#include "gameplay/Library.h"

#define REGISTER_PART0 0b111
#define REGISTER_PART1 0b111000

#define ARCADE_read(ARRAY, SONG_ID, INDEX)                               \
  u32 index = (SONG_ID * ARCADE_MAX_LEVELS + INDEX) / 2;                 \
  u32 part = (SONG_ID * ARCADE_MAX_LEVELS + INDEX) % 2;                  \
  u8 n = SAVEFILE_read8(ARRAY[index]);                                   \
  u8 value = part == 0 ? n & REGISTER_PART0 : (n & REGISTER_PART1) >> 3; \
  if (value == 0)                                                        \
    return GradeType::UNPLAYED;                                          \
                                                                         \
  return static_cast<GradeType>(value - 1);

#define ARCADE_write(ARRAY, SONG_ID, INDEX, GRADE)                \
  u32 index = (SONG_ID * ARCADE_MAX_LEVELS + INDEX) / 2;          \
  u32 part = (SONG_ID * ARCADE_MAX_LEVELS + INDEX) % 2;           \
  u8 n = SAVEFILE_read8(ARRAY[index]);                            \
  u8 value = grade == GradeType::UNPLAYED ? 0 : grade + 1;        \
  u8 updatedN = part == 0 ? (n & ~REGISTER_PART0) | value         \
                          : (n & ~REGISTER_PART1) | (value << 3); \
  SAVEFILE_write8(ARRAY[index], updatedN);

GradeType readSingle(u8 songId, u8 rawIndex) {
  ARCADE_read(SRAM->singleArcadeProgress, songId, rawIndex);
}

void writeSingle(u8 songId, u8 rawIndex, GradeType grade) {
  ARCADE_write(SRAM->singleArcadeProgress, songId, rawIndex, grade);
}

void ARCADE_initialize() {
  for (u32 i = 0; i < ARCADE_PROGRESS_SIZE; i++) {
    SAVEFILE_write8(SRAM->singleArcadeProgress[i], 0);
    SAVEFILE_write8(SRAM->doubleArcadeProgress[i], 0);
  }

  writeSingle(0, 0, GradeType::D);
}

bool ARCADE_isInitialized() {
  return readSingle(0, 0) == GradeType::D;
}

bool ARCADE_isLegacy() {
  return readSingle(0, 0) == GradeType::C;
}

GradeType ARCADE_readSingle(u8 songId, u8 levelIndex) {
  return readSingle(songId, 1 + levelIndex);
}

GradeType ARCADE_readDouble(u8 songId, u8 levelIndex) {
  ARCADE_read(SRAM->doubleArcadeProgress, songId, 1 + levelIndex);
}

void ARCADE_writeSingle(u8 songId, u8 levelIndex, GradeType grade) {
  ARCADE_write(SRAM->singleArcadeProgress, songId, 1 + levelIndex, grade);
}

void ARCADE_writeDouble(u8 songId, u8 levelIndex, GradeType grade) {
  ARCADE_write(SRAM->doubleArcadeProgress, songId, 1 + levelIndex, grade);
}

void ARCADE_migrate() {
  const GBFS_FILE* fs = find_first_gbfs_file(0);

  std::vector<std::unique_ptr<SongFile>> songFiles;
  auto library = std::unique_ptr<Library>{new Library(fs)};
  u32 librarySize = SAVEFILE_getLibrarySize();
  u32 pages =
      Div(librarySize, PAGE_SIZE) + (DivMod(librarySize, PAGE_SIZE) > 0);
  for (u32 i = 0; i < pages; i++)
    library->loadSongs(songFiles, DifficultyLevel::CRAZY, i * PAGE_SIZE);

  for (u32 i = 0; i < songFiles.size(); i++) {
    Song* song = SONG_parse(fs, songFiles[i].get());

    std::vector<GradeType> singleGrades;
    for (u32 i = 0; i < song->chartCount; i++)
      if (!song->charts[i].isDouble) {
        singleGrades.push_back(
            ARCADE_readSingle(song->id, song->charts[i].level - 1));
      }
    for (u32 i = 0; i < singleGrades.size(); i++)
      ARCADE_writeSingle(song->id, i, singleGrades[i]);

    std::vector<GradeType> doubleGrades;
    for (u32 i = 0; i < song->chartCount; i++)
      if (song->charts[i].isDouble) {
        doubleGrades.push_back(
            ARCADE_readDouble(song->id, song->charts[i].level - 1));
      }
    for (u32 i = 0; i < doubleGrades.size(); i++)
      ARCADE_writeDouble(song->id, i, doubleGrades[i]);

    SONG_free(song);
  }

  writeSingle(0, 0, GradeType::D);
}
