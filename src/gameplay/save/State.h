#ifndef STATE_H
#define STATE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct __attribute__((__packed__)) {
  u8 isPlaying;
  u8 isBoss;
} State;

#endif  // STATE_H