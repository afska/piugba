#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_memmap.h>

#include "ArcadeProgress.h"
#include "Memory.h"
#include "Mods.h"
#include "Progress.h"
#include "Settings.h"
#include "State.h"
#include "assets.h"
#include "gameplay/debug/DebugTools.h"
#include "gameplay/models/Chart.h"
#include "utils/MathUtils.h"
#include "utils/parse.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

#define ROM_ID_MASK 0xffffff00

const u32 PROGRESS_REGISTERS = 6;
const u32 PROGRESS_IMPOSSIBLE = 3;
const u32 LIBRARY_SIZE_MASK = 0x000000FF;

typedef struct __attribute__((__packed__)) {
  u32 romId;

  Settings settings;
  Mods mods;
  Memory memory;
  Progress progress[PROGRESS_REGISTERS];

  State state;

  u8 singleArcadeProgress[ARCADE_PROGRESS_SIZE];
  u8 doubleArcadeProgress[ARCADE_PROGRESS_SIZE];
} SaveFile;

#define SRAM ((SaveFile*)sram_mem)

#define SAVEFILE_read8(TARGET) (*((vu8*)&TARGET))
#define SAVEFILE_write8(DEST, VALUE) *((vu8*)&DEST) = VALUE;
#define SAVEFILE_read32(TARGET)                     \
  ((u32)(*(((volatile char*)&TARGET) + 0) +         \
         (*(((volatile char*)&TARGET) + 1) << 8) +  \
         (*(((volatile char*)&TARGET) + 2) << 16) + \
         (*(((volatile char*)&TARGET) + 3) << 24)))
#define SAVEFILE_write32(DEST, VALUE)                                   \
  *(((volatile char*)&DEST) + 0) = (((u32)(VALUE)) & 0x000000ff) >> 0;  \
  *(((volatile char*)&DEST) + 1) = (((u32)(VALUE)) & 0x0000ff00) >> 8;  \
  *(((volatile char*)&DEST) + 2) = (((u32)(VALUE)) & 0x00ff0000) >> 16; \
  *(((volatile char*)&DEST) + 3) = (((u32)(VALUE)) & 0xff000000) >> 24;

inline void SAVEFILE_resetSettings() {
  SAVEFILE_write32(SRAM->settings.audioLag, 0);
  SAVEFILE_write8(SRAM->settings.gamePosition, GamePosition::LEFT);
  SAVEFILE_write8(SRAM->settings.backgroundType, BackgroundType::FULL_BGA_DARK);
  SAVEFILE_write8(SRAM->settings.bgaDarkBlink, true);
}

inline void SAVEFILE_resetMods() {
  SAVEFILE_write8(SRAM->mods.multiplier, 3);
  SAVEFILE_write8(SRAM->mods.stageBreak, StageBreakOpts::sON);
  SAVEFILE_write8(SRAM->mods.pixelate, PixelateOpts::pOFF);
  SAVEFILE_write8(SRAM->mods.jump, JumpOpts::jOFF);
  SAVEFILE_write8(SRAM->mods.reduce, ReduceOpts::rOFF);
  SAVEFILE_write8(SRAM->mods.decolorize, DecolorizeOpts::dOFF);
  SAVEFILE_write8(SRAM->mods.randomSpeed, false);
  SAVEFILE_write8(SRAM->mods.mirrorSteps, false);
  SAVEFILE_write8(SRAM->mods.randomSteps, false);
  SAVEFILE_write8(SRAM->mods.trainingMode, TrainingModeOpts::tOFF);
}

inline void SAVEFILE_initialize(const GBFS_FILE* fs) {
  u32 romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));
  u32 librarySize = romId & LIBRARY_SIZE_MASK;
  bool isNew =
      (SAVEFILE_read32(SRAM->romId) & ROM_ID_MASK) != (romId & ROM_ID_MASK);

  // write rom id
  SAVEFILE_write32(SRAM->romId, romId);

  // create save file if needed
  if (isNew) {
    SAVEFILE_resetSettings();
    SAVEFILE_resetMods();

    SAVEFILE_write8(SRAM->memory.pageIndex, 0);
    SAVEFILE_write8(SRAM->memory.songIndex, 0);
    SAVEFILE_write8(SRAM->memory.difficultyLevel, 0);
    SAVEFILE_write8(SRAM->memory.numericLevel, 0);
    SAVEFILE_write8(SRAM->memory.isAudioLagCalibrated, false);

    SAVEFILE_write8(SRAM->progress[DifficultyLevel::NORMAL].completedSongs, 0);
    SAVEFILE_write8(SRAM->progress[DifficultyLevel::HARD].completedSongs, 0);
    SAVEFILE_write8(SRAM->progress[DifficultyLevel::CRAZY].completedSongs, 0);
    SAVEFILE_write8(
        SRAM->progress[PROGRESS_IMPOSSIBLE + DifficultyLevel::NORMAL]
            .completedSongs,
        0);
    SAVEFILE_write8(SRAM->progress[PROGRESS_IMPOSSIBLE + DifficultyLevel::HARD]
                        .completedSongs,
                    0);
    SAVEFILE_write8(SRAM->progress[PROGRESS_IMPOSSIBLE + DifficultyLevel::CRAZY]
                        .completedSongs,
                    0);

    SAVEFILE_write8(SRAM->state.isPlaying, false);
    SAVEFILE_write8(SRAM->state.gameMode, GameMode::CAMPAIGN);
  }

  // create arcade progress if needed
  if (ARCADE_readSingle(0, 0) != GradeType::C) {
    ARCADE_initialize();
    ARCADE_writeSingle(0, 0, GradeType::C);
  }

  // limit completed songs if needed
  u8 maxNormal =
      SAVEFILE_read8(SRAM->progress[DifficultyLevel::NORMAL].completedSongs);
  u8 maxHard =
      SAVEFILE_read8(SRAM->progress[DifficultyLevel::HARD].completedSongs);
  u8 maxCrazy =
      SAVEFILE_read8(SRAM->progress[DifficultyLevel::CRAZY].completedSongs);
  SAVEFILE_write8(SRAM->progress[DifficultyLevel::NORMAL].completedSongs,
                  min(maxNormal, librarySize));
  SAVEFILE_write8(SRAM->progress[DifficultyLevel::HARD].completedSongs,
                  min(maxHard, librarySize));
  SAVEFILE_write8(SRAM->progress[DifficultyLevel::CRAZY].completedSongs,
                  min(maxCrazy, librarySize));
}

inline bool SAVEFILE_isWorking(const GBFS_FILE* fs) {
  u32 romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));
  return SAVEFILE_read32(SRAM->romId) == romId;
}

inline u8 SAVEFILE_getLibrarySize() {
  return SAVEFILE_read32(SRAM->romId) & LIBRARY_SIZE_MASK;
}

inline GameMode SAVEFILE_getGameMode() {
  return static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));
}

inline u8 SAVEFILE_getMaxCompletedSongs() {
  u8 a = SAVEFILE_read8(SRAM->progress[DifficultyLevel::NORMAL].completedSongs);
  u8 b = SAVEFILE_read8(SRAM->progress[DifficultyLevel::HARD].completedSongs);
  u8 c = SAVEFILE_read8(SRAM->progress[DifficultyLevel::CRAZY].completedSongs);

  return MATH_max(a, b, c);
}

inline bool SAVEFILE_isModeUnlocked(GameMode gameMode) {
  if (ENV_ARCADE || ENV_DEVELOPMENT)
    return true;

  u8 maxCompletedSongs = SAVEFILE_getMaxCompletedSongs();

  if (gameMode == GameMode::ARCADE || gameMode == GameMode::MULTI_VS ||
      gameMode == GameMode::MULTI_COOP)
    return maxCompletedSongs >= 1;

  if (gameMode == GameMode::IMPOSSIBLE)
    return maxCompletedSongs >= SAVEFILE_getLibrarySize();

  return true;
}

inline u32 SAVEFILE_getCompletedSongsOf(DifficultyLevel difficultyLevel) {
  return SAVEFILE_read8(
      SRAM->progress[(SAVEFILE_getGameMode() == GameMode::IMPOSSIBLE) *
                         PROGRESS_IMPOSSIBLE +
                     difficultyLevel]
          .completedSongs);
}

inline DifficultyLevel SAVEFILE_getMaxLibraryType() {
  DifficultyLevel maxLevel;
  u32 max = 0;

  for (u32 i = 0; i < MAX_DIFFICULTY + 1; i++) {
    auto difficultyLevel = static_cast<DifficultyLevel>(i);
    auto completedSongs = SAVEFILE_getCompletedSongsOf(difficultyLevel);

    if (completedSongs >= max) {
      maxLevel = difficultyLevel;
      max = completedSongs;
    }
  }

  return maxLevel;
}

inline GradeType SAVEFILE_getStoryGradeOf(u8 songIndex, DifficultyLevel level) {
  auto gameMode = SAVEFILE_getGameMode();

  u32 index = (gameMode == GameMode::IMPOSSIBLE) * PROGRESS_IMPOSSIBLE + level;
  int lastIndex = SAVEFILE_read8(SRAM->progress[index].completedSongs) - 1;
  if (songIndex > lastIndex)
    return GradeType::UNPLAYED;

  return static_cast<GradeType>(
      SAVEFILE_read8(SRAM->progress[index].grades[songIndex]));
}

inline GradeType SAVEFILE_getArcadeGradeOf(u8 songId, u8 numericLevel) {
  return SAVEFILE_getGameMode() == GameMode::MULTI_COOP
             ? ARCADE_readDouble(songId, numericLevel)
             : ARCADE_readSingle(songId, numericLevel);
}

inline bool SAVEFILE_setGradeOf(u8 songIndex,
                                DifficultyLevel level,
                                u8 songId,
                                u8 numericLevel,
                                GradeType grade) {
  auto gameMode = SAVEFILE_getGameMode();

  switch (gameMode) {
    {
      case GameMode::CAMPAIGN:
      case GameMode::IMPOSSIBLE:
        u32 index =
            (gameMode == GameMode::IMPOSSIBLE) * PROGRESS_IMPOSSIBLE + level;
        int lastIndex =
            SAVEFILE_read8(SRAM->progress[index].completedSongs) - 1;
        u8 librarySize = SAVEFILE_getLibrarySize();
        bool firstTime = songIndex > lastIndex;

        if (firstTime) {
          auto nextSongIndex = (u8)min(songIndex + 1, librarySize - 1);
          auto completedSongs = (u8)min(songIndex + 1, librarySize);
          SAVEFILE_write8(SRAM->progress[index].completedSongs, completedSongs);
          SAVEFILE_write8(SRAM->memory.pageIndex, Div(nextSongIndex, 4));
          SAVEFILE_write8(SRAM->memory.songIndex, DivMod(nextSongIndex, 4));
        }

        u8 currentGrade =
            SAVEFILE_read8(SRAM->progress[index].grades[songIndex]);
        if (firstTime || grade < currentGrade)
          SAVEFILE_write8(SRAM->progress[index].grades[songIndex], grade);

        return songIndex == SAVEFILE_getLibrarySize() - 1;
    }
    {
      case GameMode::ARCADE:
      case GameMode::MULTI_VS:
        if (GameState.mods.stageBreak == StageBreakOpts::sOFF ||
            GameState.mods.trainingMode != TrainingModeOpts::tOFF)
          return false;

        u8 currentGrade = ARCADE_readSingle(songId, numericLevel);
        if (grade < currentGrade)
          ARCADE_writeSingle(songId, numericLevel, grade);

        return false;
    }
    {
      case GameMode::MULTI_COOP:
        u8 currentGrade = ARCADE_readDouble(songId, numericLevel);
        if (grade < currentGrade)
          ARCADE_writeDouble(songId, numericLevel, grade);

        return false;
    }
  }

  return false;
}

#endif  // SAVE_FILE_H
