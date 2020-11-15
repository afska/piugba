#ifndef STATE_H
#define STATE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "GameMode.h"
#include "Mods.h"
#include "gameplay/models/Song.h"

#define GAME_MAX_PLAYERS 2

const int REDUCE_MOD_POSITION_Y = 51;
const int REDUCE_MOD_SCORE_POSITION_Y = 34;
const u32 GAME_POSITION_X[] = {0, 72, 144};

typedef struct __attribute__((__packed__)) {
  u8 isPlaying;
  u8 gameMode;
} State;

typedef struct {
  int positionX[GAME_MAX_PLAYERS];
  int positionY;
  int scorePositionY;

  Mods mods;
} RAMState;

extern RAMState GameState;

void STATE_setup(Song* song, Chart* chart);

#endif  // STATE_H
