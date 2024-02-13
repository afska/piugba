#include "AdminScene.h"

#include "assets.h"
#include "gameplay/save/SaveFile.h"
#include "scenes/StartScene.h"
#include "utils/SceneUtils.h"

#define TITLE "ADMIN MENU (v1.8.4)"
#define SUBMENU_OFFSETS 0
#define SUBMENU_RESET 1
#define SUBMENU_SURE_OFFSETS 2
#define SUBMENU_SURE_ARCADE 3
#define SUBMENU_SURE_ALL 4
#define OPTIONS_COUNT_DEFAULT 6
#define OPTIONS_COUNT_OFFSETS 3
#define OPTIONS_COUNT_RESET 3
#define OPTIONS_COUNT_ARE_YOU_SURE 2

#define OPTION_NAVIGATION_STYLE 0
#define OPTION_RUMBLE 1
#define OPTION_IO_BLINK 2
#define OPTION_SRAM_BLINK 3
#define OPTION_CUSTOM_OFFSETS 4
#define OPTION_RESET_SAVE_FILE 5

AdminScene::AdminScene(std::shared_ptr<GBAEngine> engine,
                       const GBFS_FILE* fs,
                       bool withSound)
    : MenuScene(engine, fs) {
  this->withSound = withSound;
}

u16 AdminScene::getCloseKey() {
  return KEY_START | KEY_SELECT;
}

u32 AdminScene::getOptionsCount() {
  return submenu >= SUBMENU_SURE_OFFSETS ? OPTIONS_COUNT_ARE_YOU_SURE
         : submenu == SUBMENU_RESET      ? OPTIONS_COUNT_RESET
         : submenu == SUBMENU_OFFSETS    ? OPTIONS_COUNT_OFFSETS
                                         : OPTIONS_COUNT_DEFAULT;
}

void AdminScene::loadBackground(u32 id) {
  totalOffsets = OFFSET_getCount();
  VBlankIntrWait();
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_LINES_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_LINES_TILES, BG_LINES_MAP, id);
}

void AdminScene::printOptions() {
  if (withSound) {
    player_play(SOUND_STEP);
    withSound = false;
  }

  TextStream::instance().scrollNow(0, -2);

  if (submenu == SUBMENU_OFFSETS) {
    SCENE_write("CUSTOM OFFSETS", 1);

    bool offsetEditingEnabled =
        SAVEFILE_read8(SRAM->adminSettings.offsetEditingEnabled);
    printOption(0, "Offset editing", offsetEditingEnabled ? "ON" : "OFF", 4);

    SCENE_write("When editing is enabled,", 6);
    SCENE_write("press SELECT + L/R", 7);
    SCENE_write("to add -/+ 8ms offsets.", 8);
    SCENE_write("Total offsets: " + std::to_string(totalOffsets), 10);

    printOption(1, "[RESET ALL OFFSETS]", "", 13);
    printOption(2, "[BACK]", "", 15);

    return;
  } else if (submenu == SUBMENU_RESET) {
    SCENE_write("DELETE SAVE FILE", 1);

    printOption(0, "[RESET ARCADE PROGRESS]", "", 11);
    printOption(1, "[DELETE ALL SAVED DATA]", "", 13);
    printOption(2, "[BACK]", "", 15);

    return;
  } else if (submenu >= SUBMENU_SURE_OFFSETS) {
    SCENE_write("ARE YOU SURE?!", 1);

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
              navigationStyle != 1 ? "PIU" : "GBA", 4);
  printOption(OPTION_RUMBLE, "Rumble", rumble ? "ON" : "OFF", 6);
  printOption(OPTION_IO_BLINK, "I/O SD blink",
              ioBlink == 1   ? "ON BEAT"
              : ioBlink == 2 ? "ON KEY"
                             : "OFF",
              8);
  printOption(OPTION_SRAM_BLINK, "SRAM LED blink",
              sramBlink == 1   ? "ON BEAT"
              : sramBlink == 2 ? "ON HIT"
                               : "OFF",
              10);
  printOption(OPTION_CUSTOM_OFFSETS, "[CUSTOM OFFSETS]", "", 13);
  printOption(OPTION_RESET_SAVE_FILE, "[DELETE SAVE FILE]", "", 15);
}

bool AdminScene::selectOption(u32 selected, int direction) {
  if (submenu == SUBMENU_OFFSETS) {
    switch (selected) {
      case 0: {
        u8 value = SAVEFILE_read8(SRAM->adminSettings.offsetEditingEnabled);
        SAVEFILE_write8(SRAM->adminSettings.offsetEditingEnabled,
                        change(value, 2, direction));
        return true;
      }
      case 1: {
        if (direction != 0)
          return true;
        submenu = SUBMENU_SURE_OFFSETS;
        this->selected = 0;
        return true;
      }
      case 2: {
        if (direction != 0)
          return true;
        submenu = -1;
        this->selected = 0;
        return true;
      }
    }

    return true;
  }

  if (submenu == SUBMENU_RESET) {
    switch (selected) {
      case 0: {
        if (direction != 0)
          return true;
        submenu = SUBMENU_SURE_ARCADE;
        this->selected = 0;
        return true;
      }
      case 1: {
        if (direction != 0)
          return true;
        submenu = SUBMENU_SURE_ALL;
        this->selected = 0;
        return true;
      }
      case 2: {
        if (direction != 0)
          return true;
        submenu = -1;
        this->selected = 0;
        return true;
      }
    }

    return true;
  }

  if (submenu >= SUBMENU_SURE_OFFSETS) {
    if (direction != 0)
      return true;

    if (selected) {
      switch (submenu) {
        case SUBMENU_SURE_OFFSETS: {
          SAVEFILE_resetOffsets();
          SCENE_softReset();
          return true;
        }
        case SUBMENU_SURE_ARCADE: {
          SAVEFILE_resetArcade();
          SCENE_softReset();
          return true;
        }
        case SUBMENU_SURE_ALL: {
          SAVEFILE_reset();
          SCENE_softReset();
          return true;
        }
      }
    } else {
      submenu = -1;
      this->selected = 0;
    }

    return true;
  }

  switch (selected) {
    case OPTION_NAVIGATION_STYLE: {
      u8 value = SAVEFILE_read8(SRAM->adminSettings.navigationStyle);
      SAVEFILE_write8(SRAM->adminSettings.navigationStyle,
                      change(value, 2, direction));
      player_stop();
      engine->transitionIntoScene(new AdminScene(engine, fs, true),
                                  new PixelTransitionEffect());
      return false;
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
    case OPTION_CUSTOM_OFFSETS: {
      if (direction != 0)
        return true;

      submenu = SUBMENU_OFFSETS;
      this->selected = 0;
      return true;
    }
    case OPTION_RESET_SAVE_FILE: {
      if (direction != 0)
        return true;

      submenu = SUBMENU_RESET;
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
