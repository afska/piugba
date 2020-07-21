#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <libgba-sprite-engine/gba/tonc_core.h>
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

#define SAVE_MEM ((SaveFile*)sram_mem)

inline u32 SAVE_initialize(const GBFS_FILE* fs) {
  auto romId = as_le((u8*)gbfs_get_obj(fs, ROM_ID_FILE, NULL));
  return romId;
}

#endif  // SAVE_FILE_H
