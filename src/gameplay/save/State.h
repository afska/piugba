#ifndef STATE_H
#define STATE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct __attribute__((__packed__)) {
  u8 isPlaying;
} State;

typedef struct {
  bool isBoss;
} RAMState;

extern RAMState GameState;

inline void STATE_reset() {
  GameState.isBoss = false;
}

#endif  // STATE_H
