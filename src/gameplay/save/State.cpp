#include "State.h"

#include "Savefile.h"

RAMState GameState;

const u32 GAME_POSITION_X[] = {0, 72, 144};

void STATE_reset() {
  GameState.positionX =
      GAME_POSITION_X[SAVEFILE_read8(SRAM->settings.gamePosition)];
  GameState.positionY = 0;       // TODO: 51 - REDUCE MOD
  GameState.scorePositionY = 0;  // TODO: 34 - REDUCE MOD

  GameState.mods.multiplier = SAVEFILE_read8(SRAM->mods.multiplier);
  GameState.mods.stageBreak = SAVEFILE_read8(SRAM->mods.stageBreak);
  GameState.mods.pixelate = SAVEFILE_read8(SRAM->mods.pixelate);
  GameState.mods.jump = SAVEFILE_read8(SRAM->mods.jump);
  GameState.mods.reduce = SAVEFILE_read8(SRAM->mods.reduce);
  GameState.mods.negative = SAVEFILE_read8(SRAM->mods.negative);
  GameState.mods.randomSpeed = SAVEFILE_read8(SRAM->mods.randomSpeed);
  GameState.mods.mirrorSteps = SAVEFILE_read8(SRAM->mods.mirrorSteps);
  GameState.mods.randomSteps = SAVEFILE_read8(SRAM->mods.randomSteps);
  GameState.mods.extraJudgement = SAVEFILE_read8(SRAM->mods.extraJudgement);
}
