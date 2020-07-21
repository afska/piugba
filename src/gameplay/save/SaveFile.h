#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <tonc_memmap.h>

#include "Memory.h"
#include "Progress.h"
#include "Settings.h"

typedef struct {
  u32 romId;
  Settings settings;
  Memory memory;
  Progress progress;
} SaveFile;

#define SAVE_MEM ((SaveFile*)sram_mem)

#endif  // SAVE_FILE_H
