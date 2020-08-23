#include "ModsScene.h"

#include "gameplay/save/SaveFile.h"
#include "utils/SceneUtils.h"

#define TITLE "MODS"
#define OPTIONS_COUNT 11
#define OPTION_MULTIPLIER 0
#define OPTION_STAGE_BREAK 1
#define OPTION_PIXELATE 2
#define OPTION_JUMP 3
#define OPTION_REDUCE 4
#define OPTION_NEGATIVE_COLORS 5
#define OPTION_RANDOM_SPEED 6
#define OPTION_MIRROR_STEPS 7
#define OPTION_RANDOM_STEPS 8
#define OPTION_EXTRA_JUDGEMENT 9
#define OPTION_RESET 10

ModsScene::ModsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : MenuScene(engine, fs) {}

u16 ModsScene::getCloseKey() {
  return KEY_SELECT;
}

u32 ModsScene::getOptionsCount() {
  return OPTIONS_COUNT;
}

void ModsScene::printOptions() {
  SCENE_write(TITLE, 2);

  // int audioLag = (int)SAVEFILE_read32(SRAM->settings.audioLag);
  // u8 gamePosition = SAVEFILE_read8(SRAM->settings.gamePosition);
  // u8 backgroundType = SAVEFILE_read8(SRAM->settings.backgroundType);
  // bool bgaDarkBlink = SAVEFILE_read8(SRAM->settings.bgaDarkBlink);

  printOption(OPTION_MULTIPLIER, "Multiplier", "3x", 5);
  printOption(OPTION_STAGE_BREAK, "Stage break", "ON", 6);
  printOption(OPTION_PIXELATE, "Pixelate", "RANDOM", 7);
  printOption(OPTION_JUMP, "Jump", "OFF", 8);
  printOption(OPTION_REDUCE, "Reduce", "OFF", 9);
  printOption(OPTION_NEGATIVE_COLORS, "Negative colors", "TOGGLE", 10);
  printOption(OPTION_RANDOM_SPEED, "Random speed", "OFF", 11);
  printOption(OPTION_MIRROR_STEPS, "Mirror steps", "OFF", 12);
  printOption(OPTION_RANDOM_STEPS, "Random steps", "OFF", 13);
  printOption(OPTION_EXTRA_JUDGEMENT, "Extra judgement", "OFF", 14);
  printOption(OPTION_RESET, "<RESET MODS>", "", 15);
}

bool ModsScene::selectOption(u32 selected) {
  switch (selected) {}

  return true;
}
