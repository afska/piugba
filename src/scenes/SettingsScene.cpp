#include "SettingsScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/save/SaveFile.h"
#include "player/PlaybackState.h"
#include "scenes/CalibrateScene.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "player/fxes.h"
}

#define TITLE "SETTINGS"
#define OPTION_AUDIO_LAG 0
#define OPTION_SHOW_CONTROLS 1
#define OPTION_GAME_POSITION 2
#define OPTION_BACKGROUND_TYPE 3
#define OPTION_BGA_DARK_BLINK 4
#define OPTION_QUIT 5

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const int TEXT_COL_UNSELECTED = -2;
const int TEXT_COL_SELECTED = -3;
const int TEXT_COL_VALUE_MIDDLE = 20;
const u32 BUTTON_MARGIN = 8;
const u32 OPTION_MIN = 0;
const u32 OPTION_MAX = 5;
const u32 PIXEL_BLINK_LEVEL = 4;

SettingsScene::SettingsScene(std::shared_ptr<GBAEngine> engine,
                             const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> SettingsScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> SettingsScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(selectButton->get());
  sprites.push_back(backButton->get());
  sprites.push_back(nextButton->get());

  return sprites;
}

void SettingsScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();

  setUpSpritesPalette();
  setUpBackground();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  selectButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  backButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNLEFT, true, true)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNRIGHT, true, true)};

  selectButton->get()->moveTo(112,
                              GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  backButton->get()->moveTo(BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  printMenu();
}

void SettingsScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }

  processKeys(keys);
  processSelection();

  pixelBlink->tick();
  selectButton->tick();
  backButton->tick();
  nextButton->tick();
}

void SettingsScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal)));
}

void SettingsScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_LINES_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_LINES_TILES, BG_LINES_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void SettingsScene::processKeys(u16 keys) {
  selectButton->setIsPressed(KEY_CENTER(keys));
  backButton->setIsPressed(KEY_DOWNLEFT(keys));
  nextButton->setIsPressed(KEY_DOWNRIGHT(keys));

  if (keys & KEY_START) {
    fxes_stop();
    engine->transitionIntoScene(new SelectionScene(engine, fs),
                                new FadeOutScene(2));
  }
}

void SettingsScene::processSelection() {
  if (selectButton->hasBeenPressedNow())
    select();

  if (nextButton->hasBeenPressedNow())
    move(1);

  if (backButton->hasBeenPressedNow())
    move(-1);
}

void SettingsScene::printMenu() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  BACKGROUND_write(TITLE, 2);

  int audioLag = (int)SAVEFILE_read32(SRAM->settings.audioLag);
  bool showControls = SAVEFILE_read8(SRAM->settings.showControls);
  u8 gamePosition = SAVEFILE_read8(SRAM->settings.gamePosition);
  u8 backgroundType = SAVEFILE_read8(SRAM->settings.backgroundType);
  bool bgaDarkBlink = SAVEFILE_read8(SRAM->settings.bgaDarkBlink);

  printOption(OPTION_AUDIO_LAG, "Audio lag", std::to_string(audioLag), 5);
  printOption(OPTION_SHOW_CONTROLS, "Show controls",
              showControls ? "ON" : "OFF", 7);
  printOption(
      OPTION_GAME_POSITION, "Game position",
      gamePosition == 0 ? "LEFT" : gamePosition == 1 ? "CENTER" : "RIGHT", 9);
  printOption(OPTION_BACKGROUND_TYPE, "Background type",
              backgroundType == 0
                  ? "RAW"
                  : backgroundType == 1 ? "HALF_DARK" : "FULL_DARK",
              11);
  if (backgroundType > 0)
    printOption(OPTION_BGA_DARK_BLINK, "Background blink",
                bgaDarkBlink ? "ON" : "OFF", 13);
  else
    printOption(OPTION_BGA_DARK_BLINK, "----------------",
                bgaDarkBlink ? "---" : "---", 13);
  printOption(OPTION_QUIT, "QUIT GAME", "", 15);
}

void SettingsScene::printOption(u32 id,
                                std::string name,
                                std::string value,
                                u32 row) {
  bool isActive = selected == id;
  TextStream::instance().setText(
      (isActive ? ">" : "") + name, row,
      isActive ? TEXT_COL_SELECTED : TEXT_COL_UNSELECTED);

  if (value.length() > 0) {
    auto valueString = "<" + value + ">";
    TextStream::instance().setText(
        valueString, row, TEXT_COL_VALUE_MIDDLE - valueString.length() / 2);
  }
}

void SettingsScene::move(int direction) {
  if (selected == OPTION_MAX && direction > 0)
    selected = OPTION_MIN;
  else if (selected == OPTION_MIN && direction < 0)
    selected = OPTION_MAX;
  else
    selected += direction;

  fxes_play(SOUND_STEP);
  pixelBlink->blink();
  printMenu();
}

void SettingsScene::select() {
  fxes_play(SOUND_STEP);
  pixelBlink->blink();

  switch (selected) {
    case OPTION_AUDIO_LAG: {
      fxes_stop();
      engine->transitionIntoScene(new CalibrateScene(engine, fs),
                                  new FadeOutScene(2));
      break;
    }
    case OPTION_SHOW_CONTROLS: {
      bool showControls = SAVEFILE_read8(SRAM->settings.showControls);
      SAVEFILE_write8(SRAM->settings.showControls, !showControls);
      printMenu();
      break;
    }
    case OPTION_GAME_POSITION: {
      u8 gamePosition = SAVEFILE_read8(SRAM->settings.gamePosition);
      SAVEFILE_write8(SRAM->settings.gamePosition,
                      gamePosition == 0 ? 1 : gamePosition == 1 ? 2 : 0);
      printMenu();
      break;
    }
    case OPTION_BACKGROUND_TYPE: {
      u8 backgroundType = SAVEFILE_read8(SRAM->settings.backgroundType);
      SAVEFILE_write8(SRAM->settings.backgroundType,
                      backgroundType == 0 ? 1 : backgroundType == 1 ? 2 : 0);
      if (backgroundType == 0)
        SAVEFILE_write8(SRAM->settings.bgaDarkBlink, true);
      printMenu();
      break;
    }
    case OPTION_BGA_DARK_BLINK: {
      bool bgaDarkBlink = SAVEFILE_read8(SRAM->settings.bgaDarkBlink);
      SAVEFILE_write8(SRAM->settings.bgaDarkBlink, !bgaDarkBlink);
      printMenu();
      break;
    }
    case OPTION_QUIT: {
      fxes_stop();
      engine->transitionIntoScene(
          new SelectionScene(engine, fs),  // TODO: StartScene
          new FadeOutScene(2));
      break;
    }
  }
}
