#include "ModsScene.h"

#include "assets.h"
#include "gameplay/multiplayer/Syncer.h"
#include "gameplay/save/SaveFile.h"
#include "utils/SceneUtils.h"

#define TITLE "MODS"
#define OPTIONS_COUNT 11
#define OPTION_MULTIPLIER 0
#define OPTION_STAGE_BREAK 1
#define OPTION_PIXELATE 2
#define OPTION_JUMP 3
#define OPTION_REDUCE 4
#define OPTION_DECOLORIZE 5
#define OPTION_RANDOM_SPEED 6
#define OPTION_MIRROR_STEPS 7
#define OPTION_RANDOM_STEPS 8
#define OPTION_TRAINING_MODE 9
#define OPTION_RESET 10

const u32 TEXT_BLEND_ALPHA = 12;

ModsScene::ModsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : MenuScene(engine, fs) {}

u16 ModsScene::getCloseKey() {
  return KEY_SELECT;
}

u32 ModsScene::getOptionsCount() {
  return OPTIONS_COUNT;
}

void ModsScene::loadBackground(u32 id) {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_WALL_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_WALL_TILES, BG_WALL_MAP, id);

  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);
}

void ModsScene::printOptions() {
  SCENE_write(TITLE, 2);

  u8 multiplier = SAVEFILE_read8(SRAM->mods.multiplier);
  u8 stageBreak = SAVEFILE_read8(SRAM->mods.stageBreak);
  u8 pixelate = SAVEFILE_read8(SRAM->mods.pixelate);
  u8 jump = SAVEFILE_read8(SRAM->mods.jump);
  u8 reduce = SAVEFILE_read8(SRAM->mods.reduce);
  u8 decolorize = SAVEFILE_read8(SRAM->mods.decolorize);
  bool randomSpeed = SAVEFILE_read8(SRAM->mods.randomSpeed);
  bool mirrorSteps = SAVEFILE_read8(SRAM->mods.mirrorSteps);
  bool randomSteps = SAVEFILE_read8(SRAM->mods.randomSteps);
  u8 trainingMode = SAVEFILE_read8(SRAM->mods.trainingMode);

  printOption(OPTION_MULTIPLIER, "Multiplier", std::to_string(multiplier) + "x",
              4);
  printOption(OPTION_STAGE_BREAK, "Stage break",
              stageBreak == 0 ? "ON" : stageBreak == 1 ? "OFF" : "SUDDEN DEATH",
              5);
  printOption(OPTION_PIXELATE, "Pixelate",
              pixelate == 0
                  ? "OFF"  // ° ͜ʖ ͡°)
                  : pixelate == 1
                        ? "LIFE"  // ° ͜ʖ ͡°)
                        : pixelate == 2
                              ? "FIXED"  // ° ͜ʖ ͡°)
                              : pixelate == 3
                                    ? "BLINK IN"  // ° ͜ʖ ͡°)
                                    : pixelate == 4 ? "BLINK OUT" : "RANDOM",
              6);
  if (isSinglePlayerDouble()) {
    printOption(OPTION_JUMP, "Jump", "---", 7);
  } else
    printOption(OPTION_JUMP, "Jump",
                jump == 0 ? "OFF" : jump == 1 ? "LINEAR" : "RANDOM", 7);
  printOption(OPTION_REDUCE, "Reduce",
              reduce == 0 ? "OFF"
                          : reduce == 1 ? "LINEAR"
                                        : reduce == 2 ? "FIXED"
                                                      : reduce == 3 ? "RANDOM"
                                                                    : "MICRO",
              8);
  printOption(OPTION_DECOLORIZE, "Decolorize",
              decolorize == 0
                  ? "OFF"  // ° ͜ʖ ͡°)
                  : decolorize == 1
                        ? "INVERT"  // ° ͜ʖ ͡°)
                        : decolorize == 2
                              ? "GRAY"  // ° ͜ʖ ͡°)
                              : decolorize == 3
                                    ? "RED"  // ° ͜ʖ ͡°)
                                    : decolorize == 4 ? "GREEN" : "BLUE",
              9);
  printOption(OPTION_RANDOM_SPEED, "Random speed", randomSpeed ? "ON" : "OFF",
              10);
  printOption(OPTION_MIRROR_STEPS, "Mirror steps", mirrorSteps ? "ON" : "OFF",
              11);
  if (isSinglePlayerDouble())
    printOption(OPTION_RANDOM_STEPS, "Random steps", "---", 12);
  else
    printOption(OPTION_RANDOM_STEPS, "Random steps", randomSteps ? "ON" : "OFF",
                12);
  printOption(OPTION_TRAINING_MODE, "Training mode",
              trainingMode == 0 ? "OFF" : trainingMode == 1 ? "ON" : "SILENT",
              13);
  printOption(OPTION_RESET, "[RESET MODS]", "", 15);
}

bool ModsScene::selectOption(u32 selected) {
  switch (selected) {
    case OPTION_MULTIPLIER: {
      u8 multiplier = SAVEFILE_read8(SRAM->mods.multiplier);
      if (multiplier == ARROW_MAX_MULTIPLIER)
        multiplier = 0;
      multiplier++;

      SAVEFILE_write8(SRAM->mods.multiplier, multiplier);
      return true;
    }
    case OPTION_STAGE_BREAK: {
      u8 stageBreak = SAVEFILE_read8(SRAM->mods.stageBreak);
      SAVEFILE_write8(SRAM->mods.stageBreak, increment(stageBreak, 3));
      return true;
    }
    case OPTION_PIXELATE: {
      u8 pixelate = SAVEFILE_read8(SRAM->mods.pixelate);
      SAVEFILE_write8(SRAM->mods.pixelate, increment(pixelate, 6));
      return true;
    }
    case OPTION_JUMP: {
      if (isSinglePlayerDouble())
        return true;

      u8 jump = SAVEFILE_read8(SRAM->mods.jump);
      SAVEFILE_write8(SRAM->mods.jump, increment(jump, 3));
      return true;
    }
    case OPTION_REDUCE: {
      u8 reduce = SAVEFILE_read8(SRAM->mods.reduce);
      SAVEFILE_write8(SRAM->mods.reduce, increment(reduce, 5));
      return true;
    }
    case OPTION_DECOLORIZE: {
      u8 decolorize = SAVEFILE_read8(SRAM->mods.decolorize);
      SAVEFILE_write8(SRAM->mods.decolorize, increment(decolorize, 6));
      return true;
    }
    case OPTION_RANDOM_SPEED: {
      bool randomSpeed = SAVEFILE_read8(SRAM->mods.randomSpeed);
      SAVEFILE_write8(SRAM->mods.randomSpeed, !randomSpeed);
      return true;
    }
    case OPTION_MIRROR_STEPS: {
      bool mirrorSteps = SAVEFILE_read8(SRAM->mods.mirrorSteps);
      SAVEFILE_write8(SRAM->mods.mirrorSteps, !mirrorSteps);
      return true;
    }
    case OPTION_RANDOM_STEPS: {
      if (isSinglePlayerDouble())
        return true;

      bool randomSteps = SAVEFILE_read8(SRAM->mods.randomSteps);
      SAVEFILE_write8(SRAM->mods.randomSteps, !randomSteps);
      return true;
    }
    case OPTION_TRAINING_MODE: {
      u8 trainingMode = SAVEFILE_read8(SRAM->mods.trainingMode);
      SAVEFILE_write8(SRAM->mods.trainingMode, increment(trainingMode, 3));
      return true;
    }
    case OPTION_RESET: {
      SAVEFILE_resetMods();
      return true;
    }
  }

  return true;
}
