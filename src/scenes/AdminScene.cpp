#include "AdminScene.h"

#include "assets.h"
#include "gameplay/save/SaveFile.h"
#include "scenes/StartScene.h"
#include "utils/SceneUtils.h"

#define TITLE "ADMIN MENU (v1.7.0)"
#define OPTIONS_COUNT 6
#define OPTION_NAVIGATION_STYLE 0
#define OPTION_RUMBLE 1
#define OPTION_IO_BLINK 2
#define OPTION_SRAM_BLINK 3
#define OPTION_RESET_ARCADE_PROGRESS 4
#define OPTION_DELETE_ALL_DATA 5

AdminScene::AdminScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : MenuScene(engine, fs) {}

u16 AdminScene::getCloseKey() {
  return KEY_START | KEY_SELECT;
}

u32 AdminScene::getOptionsCount() {
  return areYouSure > -1 ? 2 : OPTIONS_COUNT;
}

void AdminScene::loadBackground(u32 id) {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_LINES_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_LINES_TILES, BG_LINES_MAP, id);
}

void AdminScene::printOptions() {
  TextStream::instance().scroll(0, -2);

  if (areYouSure > -1) {
    SCENE_write("ARE YOU SURE?!", 2);

    printOption(0, "[NO]", "", 13);
    printOption(1, "[YES]", "", 15);

    return;
  }

  SCENE_write(TITLE, 1);

  u8 navigationStyle = SAVEFILE_read8(SRAM->adminSettings.navigationStyle);
  u8 rumble = SAVEFILE_read8(SRAM->adminSettings.rumble);
  u8 ioBlink = SAVEFILE_read8(SRAM->adminSettings.ioBlink);
  u8 sramBlink = SAVEFILE_read8(SRAM->adminSettings.sramBlink);

  printOption(OPTION_NAVIGATION_STYLE, "Navigation style",
              navigationStyle == 1 ? "GBA" : "PIU", 4);
  printOption(OPTION_RUMBLE, "Rumble", rumble == 0 ? "OFF" : "ON", 6);
  printOption(OPTION_IO_BLINK, "I/O SD blink",
              ioBlink == 0   ? "OFF"
              : ioBlink == 1 ? "ON BEAT"
                             : "ON KEY",
              8);
  printOption(OPTION_SRAM_BLINK, "SRAM LED blink",
              sramBlink == 0   ? "OFF"
              : sramBlink == 1 ? "ON BEAT"
                               : "ON HIT",
              10);
  printOption(OPTION_RESET_ARCADE_PROGRESS, "  [RESET ARCADE PROGRESS]", "",
              13);
  printOption(OPTION_DELETE_ALL_DATA, "  [DELETE ALL SAVED DATA]", "", 15);
}

bool AdminScene::selectOption(u32 selected, int direction) {
  if (areYouSure > -1) {
    if (direction != 0)
      return true;

    if (selected) {
      switch (areYouSure) {
        case OPTION_RESET_ARCADE_PROGRESS: {
          SAVEFILE_resetArcade();
          SCENE_softReset();
          return true;
        }
        case OPTION_DELETE_ALL_DATA: {
          SAVEFILE_reset();
          SCENE_softReset();
          return true;
        }
      }
    }

    areYouSure = -1;
    this->selected = 0;
    return true;
  }

  switch (selected) {
    case OPTION_NAVIGATION_STYLE: {
      u8 value = SAVEFILE_read8(SRAM->adminSettings.navigationStyle);
      SAVEFILE_write8(SRAM->adminSettings.navigationStyle,
                      change(value, 2, direction));
      return true;
    }
    case OPTION_RUMBLE: {
      u8 value = SAVEFILE_read8(SRAM->adminSettings.rumble);
      SAVEFILE_write8(SRAM->adminSettings.rumble, change(value, 2, direction));
      return true;
    }
    case OPTION_IO_BLINK: {
      u8 value = SAVEFILE_read8(SRAM->adminSettings.ioBlink);
      SAVEFILE_write8(SRAM->adminSettings.ioBlink, change(value, 3, direction));
      return true;
    }
    case OPTION_SRAM_BLINK: {
      u8 value = SAVEFILE_read8(SRAM->adminSettings.sramBlink);
      SAVEFILE_write8(SRAM->adminSettings.sramBlink,
                      change(value, 3, direction));
      return true;
    }
    case OPTION_RESET_ARCADE_PROGRESS: {
      if (direction != 0)
        return true;

      areYouSure = OPTION_RESET_ARCADE_PROGRESS;
      this->selected = 0;
      return true;
    }
    case OPTION_DELETE_ALL_DATA: {
      if (direction != 0)
        return true;

      areYouSure = OPTION_DELETE_ALL_DATA;
      this->selected = 0;
      return true;
    }
  }

  return true;
}

void AdminScene::close() {
  player_stop();
  engine->transitionIntoScene(new StartScene(engine, fs),
                              new PixelTransitionEffect());
}
