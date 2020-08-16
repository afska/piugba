#include "State.h"

#include "Savefile.h"

RAMState GameState;

const u32 GAME_POSITION_X[] = {0, 72, 144};

void STATE_reset() {
  GameState.isBoss = false;
  GameState.positionX =
      GAME_POSITION_X[SAVEFILE_read8(SRAM->settings.gamePosition)];
  GameState.positionY = 0;
}
