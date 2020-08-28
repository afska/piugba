#include "State.h"

#include "Savefile.h"

RAMState GameState;

void STATE_setup(Song* song) {
  auto gameMode = static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));
  if (song == NULL)
    gameMode = GameMode::ARCADE;

  GameState.positionX = 0;
  GameState.positionY = 0;
  GameState.scorePositionY = 0;

  GameState.mods.multiplier = SAVEFILE_read8(SRAM->mods.multiplier);

  switch (gameMode) {
    case GameMode::CAMPAIGN: {
      GameState.mods.stageBreak = true;
      GameState.mods.pixelate = static_cast<PixelateOpts>(song->pixelate);
      GameState.mods.jump = song->jump;
      GameState.mods.reduce = static_cast<ReduceOpts>(song->reduce);
      GameState.mods.negative = song->negativeColors;
      GameState.mods.randomSpeed = song->randomSpeed;
      GameState.mods.mirrorSteps = false;
      GameState.mods.randomSteps = false;
      GameState.mods.extraJudgement = false;
      break;
    }
    case GameMode::ARCADE: {
      GameState.mods.stageBreak = SAVEFILE_read8(SRAM->mods.stageBreak);
      GameState.mods.pixelate =
          static_cast<PixelateOpts>(SAVEFILE_read8(SRAM->mods.pixelate));
      GameState.mods.jump = SAVEFILE_read8(SRAM->mods.jump);
      GameState.mods.reduce =
          static_cast<ReduceOpts>(SAVEFILE_read8(SRAM->mods.reduce));
      GameState.mods.negative = SAVEFILE_read8(SRAM->mods.negative);
      GameState.mods.randomSpeed = SAVEFILE_read8(SRAM->mods.randomSpeed);
      GameState.mods.mirrorSteps = SAVEFILE_read8(SRAM->mods.mirrorSteps);
      GameState.mods.randomSteps = SAVEFILE_read8(SRAM->mods.randomSteps);
      GameState.mods.extraJudgement = SAVEFILE_read8(SRAM->mods.extraJudgement);
      break;
    }
    case GameMode::IMPOSSIBLE: {
      GameState.mods.stageBreak = true;
      GameState.mods.pixelate = PixelateOpts::pBLINK_IN;
      GameState.mods.jump = true;
      GameState.mods.reduce = ReduceOpts::rOFF;
      GameState.mods.negative = true;
      GameState.mods.randomSpeed = false;
      GameState.mods.mirrorSteps = true;
      GameState.mods.randomSteps = false;
      GameState.mods.extraJudgement = true;
      break;
    }
  }

  if (!GameState.mods.jump)
    GameState.positionX =
        GAME_POSITION_X[SAVEFILE_read8(SRAM->settings.gamePosition)];

  if (GameState.mods.reduce != ReduceOpts::rOFF) {
    GameState.positionY = REDUCE_MOD_POSITION_Y;
    GameState.scorePositionY = REDUCE_MOD_SCORE_POSITION_Y;
  }
}
