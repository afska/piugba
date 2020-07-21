#ifndef MEMORY_H
#define MEMORY_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct {
  u8 lastPage;
  u8 lastSong;
  u8 multiplier;
} Memory;

#endif  // MEMORY_H
