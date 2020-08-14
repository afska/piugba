#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_memmap.h>

#include "Memory.h"
#include "Progress.h"
#include "Settings.h"
#include "State.h"
#include "assets.h"
#include "objects/Difficulty.h"
#include "utils/parse.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

const u32 PROGRESS_REGISTERS = 6;

typedef struct {
  u32 romId;

  Settings settings;
  Memory memory;
  Progress progress[PROGRESS_REGISTERS];

  State state;
} SaveFile;

#define SRAM ((SaveFile*)sram_mem)

#define SAVEFILE_read8(TARGET) *((u8*)&TARGET)
#define SAVEFILE_write8(DEST, VALUE) *((u8*)&DEST) = VALUE;
#define SAVEFILE_read32(TARGET)                                    \
  (u32)(*(((char*)&TARGET) + 0) + (*(((char*)&TARGET) + 1) << 8) + \
        (*(((char*)&TARGET) + 2) << 16) + (*(((char*)&TARGET) + 3) << 24))
#define SAVEFILE_write32(DEST, VALUE)                        \
  *(((char*)&DEST) + 0) = (((u32)VALUE) & 0x000000ff) >> 0;  \
  *(((char*)&DEST) + 1) = (((u32)VALUE) & 0x0000ff00) >> 8;  \
  *(((char*)&DEST) + 2) = (((u32)VALUE) & 0x00ff0000) >> 16; \
  *(((char*)&DEST) + 3) = (((u32)VALUE) & 0xff000000) >> 24;

inline void SAVEFILE_initialize(const GBFS_FILE* fs) {
  u32 romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));

  if (SAVEFILE_read32(SRAM->romId) != romId) {
    SAVEFILE_write32(SRAM->romId, romId);

    SAVEFILE_write32(SRAM->settings.audioLag, 0);
    SAVEFILE_write8(SRAM->settings.showControls, 1);
    SAVEFILE_write8(SRAM->settings.gamePosition, GamePosition::LEFT);
    SAVEFILE_write8(SRAM->settings.backgroundType,
                    BackgroundType::HALF_BGA_DARK);
    SAVEFILE_write8(SRAM->settings.bgaDarkBlink, true);

    SAVEFILE_write8(SRAM->memory.pageIndex, 0);
    SAVEFILE_write8(SRAM->memory.songIndex, 0);
    SAVEFILE_write8(SRAM->memory.difficultyLevel, 0);
    SAVEFILE_write8(SRAM->memory.multiplier, 3);

    u32 i;
    for (i = 0; i < PROGRESS_REGISTERS; i++) {
      SAVEFILE_write32(SRAM->progress[i].completedSongs, 0);
    }

    SAVEFILE_write8(SRAM->state.isPlaying, 0);
    SAVEFILE_write8(SRAM->state.pixelate, 0);
  }
}

inline GradeType SAVEFILE_getGradeOf(u8 songIndex, DifficultyLevel level) {
  auto lastIndex = SAVEFILE_read8(SRAM->progress[level].completedSongs) - 1;
  if (songIndex > lastIndex)
    return GradeType::UNPLAYED;

  return static_cast<GradeType>(
      SAVEFILE_read8(SRAM->progress[level].grades[songIndex]));
}

inline void SAVEFILE_setGradeOf(u8 songIndex,
                                DifficultyLevel level,
                                GradeType grade) {
  auto lastIndex = SAVEFILE_read8(SRAM->progress[level].completedSongs) - 1;
  if (songIndex > lastIndex) {
    SAVEFILE_write32(SRAM->progress[level].completedSongs, songIndex + 1);
  }

  SAVEFILE_write8(SRAM->progress[level].grades[songIndex], grade);
}

#endif  // SAVE_FILE_H
