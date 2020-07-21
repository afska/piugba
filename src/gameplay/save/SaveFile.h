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

#define _ ((SaveFile*)sram_mem)

#define SAVEFILE_READ8(TARGET) *((u8*)&TARGET)
#define SAVEFILE_WRITE8(DEST, VALUE) *((u8*)&DEST) = VALUE;
#define SAVEFILE_READ32(TARGET)                                    \
  (u32)(*(((char*)&TARGET) + 0) + (*(((char*)&TARGET) + 1) << 8) + \
        (*(((char*)&TARGET) + 2) << 16) + (*(((char*)&TARGET) + 3) << 24))
#define SAVEFILE_WRITE32(DEST, VALUE)                        \
  *(((char*)&DEST) + 0) = (((u32)VALUE) & 0x000000ff) >> 0;  \
  *(((char*)&DEST) + 1) = (((u32)VALUE) & 0x0000ff00) >> 8;  \
  *(((char*)&DEST) + 2) = (((u32)VALUE) & 0x00ff0000) >> 16; \
  *(((char*)&DEST) + 3) = (((u32)VALUE) & 0xff000000) >> 24;

inline void SAVEFILE_initialize(const GBFS_FILE* fs) {
  u32 romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));

  if (SAVEFILE_READ32(_->romId) != romId) {
    SAVEFILE_WRITE32(_->romId, romId);

    SAVEFILE_WRITE32(_->settings.audioLag, 0);
    SAVEFILE_WRITE8(_->settings.pixelize, 0);
    SAVEFILE_WRITE8(_->settings.holderPosition, HolderPosition::LEFT);
    SAVEFILE_WRITE8(_->settings.backgroundType, BackgroundType::HALF_BGA_DARK);
    SAVEFILE_WRITE8(_->settings.bgaDarkBlink, true);

    SAVEFILE_WRITE8(_->memory.lastPage, 0);
    SAVEFILE_WRITE8(_->memory.lastSong, 0);
    SAVEFILE_WRITE8(_->memory.multiplier, 3);

    SAVEFILE_WRITE32(_->progress.completedSongs, 0);
  }
}

#endif  // SAVE_FILE_H
