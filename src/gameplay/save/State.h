#ifndef STATE_H
#define STATE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct __attribute__((__packed__)) {
  u8 isPlaying;
} State;

typedef struct {
  bool isBoss;
  int positionX;
  int positionY;
  int scorePositionY;
} RAMState;

extern RAMState GameState;

void STATE_reset();

#endif  // STATE_H
