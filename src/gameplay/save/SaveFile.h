#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <libgba-sprite-engine/gba/tonc_core.h>  // TODO: Stop using headers from libgba-sprite-engine
#include <tonc_memmap.h>

#include "Memory.h"
#include "Progress.h"
#include "Settings.h"
#include "assets.h"
#include "utils/parse.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

typedef struct {
  u32 romId;
  Settings settings;
  Memory memory;
  Progress progress;
} SaveFile;

#define SAVEFILE ((SaveFile*)sram_mem)

inline void SAVEFILE_initialize(const GBFS_FILE* fs) {
  auto romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));

  if (SAVEFILE->romId != romId) {
    // TODO: This doesn't work. SRAM memory is 8-bit write only. Create a
    // singleton class with getters and setters
    SAVEFILE->romId = romId;

    SAVEFILE->settings.audioLag = 0;
    SAVEFILE->settings.pixelize = false;
    SAVEFILE->settings.holderPosition = 0;
    SAVEFILE->settings.backgroundType = BackgroundType::HALF_BGA_DARK;
    SAVEFILE->settings.bgaDarkBlink = true;

    SAVEFILE->memory.lastPage = 0;
    SAVEFILE->memory.lastSong = 0;
    SAVEFILE->memory.multiplier = 3;

    SAVEFILE->progress.completedSongs = 0;
  }
}

#endif  // SAVE_FILE_H
