#include "State.h"

#include "Savefile.h"

RAMState GameState;

void STATE_setup(Song* song, Chart* chart) {
  auto gameMode = static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));
  if (song == NULL)
    gameMode = GameMode::ARCADE;

  GameState.positionX = 0;
  GameState.positionY = 0;
  GameState.scorePositionY = 0;

  GameState.mods.multiplier = SAVEFILE_read8(SRAM->mods.multiplier);

  switch (gameMode) {
    case GameMode::CAMPAIGN: {
      GameState.mods.stageBreak =
          !ENV_DEVELOPMENT || ((~REG_KEYS & KEY_ANY) & KEY_SELECT)
              ? StageBreakOpts::sON
              : StageBreakOpts::sOFF;
      GameState.mods.pixelate = PixelateOpts::pOFF;
      GameState.mods.jump = JumpOpts::jOFF;
      GameState.mods.reduce = ReduceOpts::rOFF;
      GameState.mods.decolorize = DecolorizeOpts::dOFF;
      GameState.mods.randomSpeed = false;
      GameState.mods.mirrorSteps = false;
      GameState.mods.randomSteps = false;
      GameState.mods.trainingMode = TrainingModeOpts::tOFF;

      if (song->applyTo[chart->difficulty]) {
        GameState.mods.pixelate = static_cast<PixelateOpts>(song->pixelate);
        GameState.mods.jump = static_cast<JumpOpts>(song->jump);
        GameState.mods.reduce = static_cast<ReduceOpts>(song->reduce);
        GameState.mods.decolorize =
            static_cast<DecolorizeOpts>(song->decolorize);
        GameState.mods.randomSpeed = song->randomSpeed;
      }

      break;
    }
    case GameMode::ARCADE: {
      GameState.mods.stageBreak =
          static_cast<StageBreakOpts>(SAVEFILE_read8(SRAM->mods.stageBreak));
      GameState.mods.pixelate =
          static_cast<PixelateOpts>(SAVEFILE_read8(SRAM->mods.pixelate));
      GameState.mods.jump =
          static_cast<JumpOpts>(SAVEFILE_read8(SRAM->mods.jump));
      GameState.mods.reduce =
          static_cast<ReduceOpts>(SAVEFILE_read8(SRAM->mods.reduce));
      GameState.mods.decolorize =
          static_cast<DecolorizeOpts>(SAVEFILE_read8(SRAM->mods.decolorize));
      GameState.mods.randomSpeed = SAVEFILE_read8(SRAM->mods.randomSpeed);
      GameState.mods.mirrorSteps = SAVEFILE_read8(SRAM->mods.mirrorSteps);
      GameState.mods.randomSteps = SAVEFILE_read8(SRAM->mods.randomSteps);
      GameState.mods.trainingMode = static_cast<TrainingModeOpts>(
          SAVEFILE_read8(SRAM->mods.trainingMode));
      break;
    }
    case GameMode::IMPOSSIBLE: {
      GameState.mods.stageBreak =
          !ENV_DEVELOPMENT || ((~REG_KEYS & KEY_ANY) & KEY_SELECT)
              ? StageBreakOpts::sON
              : StageBreakOpts::sOFF;
      GameState.mods.pixelate = PixelateOpts::pRANDOM;
      GameState.mods.jump = JumpOpts::jLINEAR;
      GameState.mods.reduce = ReduceOpts::rLINEAR;
      GameState.mods.decolorize = DecolorizeOpts::dOFF;
      GameState.mods.randomSpeed = false;
      GameState.mods.mirrorSteps = true;
      GameState.mods.randomSteps = false;
      GameState.mods.trainingMode = TrainingModeOpts::tOFF;
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
