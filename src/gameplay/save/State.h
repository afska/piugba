#ifndef STATE_H
#define STATE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Mods.h"

enum GameMode : u8 { CAMPAIGN, ARCADE, IMPOSSIBLE };

typedef struct __attribute__((__packed__)) {
  u8 isPlaying;
  u8 gameMode;
} State;

typedef struct {
  int positionX;
  int positionY;
  int scorePositionY;

  Mods mods;
} RAMState;

extern RAMState GameState;

void STATE_reset();

#endif  // STATE_H
