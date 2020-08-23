#include "SettingsScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "CalibrateScene.h"
#include "StartScene.h"
#include "gameplay/save/SaveFile.h"
#include "utils/SceneUtils.h"

#define TITLE "SETTINGS"
#define OPTIONS_COUNT 6
#define OPTION_AUDIO_LAG 0
#define OPTION_GAME_POSITION 1
#define OPTION_BACKGROUND_TYPE 2
#define OPTION_BGA_DARK_BLINK 3
#define OPTION_RESET 4
#define OPTION_QUIT 5

SettingsScene::SettingsScene(std::shared_ptr<GBAEngine> engine,
                             const GBFS_FILE* fs)
    : MenuScene(engine, fs) {}

u16 SettingsScene::getCloseKey() {
  return KEY_START;
}

u32 SettingsScene::getOptionsCount() {
  return OPTIONS_COUNT;
}

void SettingsScene::loadBackground(u32 id) {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_LINES_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_LINES_TILES, BG_LINES_MAP, id);
}

void SettingsScene::printOptions() {
  SCENE_write(TITLE, 2);

  int audioLag = (int)SAVEFILE_read32(SRAM->settings.audioLag);
  u8 gamePosition = SAVEFILE_read8(SRAM->settings.gamePosition);
  u8 backgroundType = SAVEFILE_read8(SRAM->settings.backgroundType);
  bool bgaDarkBlink = SAVEFILE_read8(SRAM->settings.bgaDarkBlink);

  printOption(OPTION_AUDIO_LAG, "Audio lag", std::to_string(audioLag), 5);
  printOption(
      OPTION_GAME_POSITION, "Game position",
      gamePosition == 0 ? "LEFT" : gamePosition == 1 ? "CENTER" : "RIGHT", 7);
  printOption(OPTION_BACKGROUND_TYPE, "Background type",
              backgroundType == 0
                  ? "RAW"
                  : backgroundType == 1 ? "HALF_DARK" : "FULL_DARK",
              9);
  if (backgroundType > 0)
    printOption(OPTION_BGA_DARK_BLINK, "Background blink",
                bgaDarkBlink ? "ON" : "OFF", 11);
  else
    printOption(OPTION_BGA_DARK_BLINK, "----------------",
                bgaDarkBlink ? "---" : "---", 11);
  printOption(OPTION_RESET, "[RESET OPTIONS]", "", 13);
  printOption(OPTION_QUIT, "[QUIT TO MAIN MENU]", "", 15);
}

bool SettingsScene::selectOption(u32 selected) {
  switch (selected) {
    case OPTION_AUDIO_LAG: {
      engine->transitionIntoScene(new CalibrateScene(engine, fs, NULL),
                                  new FadeOutScene(2));
      return false;
    }
    case OPTION_GAME_POSITION: {
      u8 gamePosition = SAVEFILE_read8(SRAM->settings.gamePosition);
      SAVEFILE_write8(SRAM->settings.gamePosition, increment(gamePosition, 3));
      return true;
    }
    case OPTION_BACKGROUND_TYPE: {
      u8 backgroundType = SAVEFILE_read8(SRAM->settings.backgroundType);
      SAVEFILE_write8(SRAM->settings.backgroundType,
                      increment(backgroundType, 3));
      if (backgroundType == 0)
        SAVEFILE_write8(SRAM->settings.bgaDarkBlink, true);
      return true;
    }
    case OPTION_BGA_DARK_BLINK: {
      bool bgaDarkBlink = SAVEFILE_read8(SRAM->settings.bgaDarkBlink);
      SAVEFILE_write8(SRAM->settings.bgaDarkBlink, !bgaDarkBlink);
      return true;
    }
    case OPTION_RESET: {
      SAVEFILE_resetSettings();
      return true;
    }
    case OPTION_QUIT: {
      engine->transitionIntoScene(new StartScene(engine, fs),
                                  new FadeOutScene(2));
      return false;
    }
  }

  return true;
}
