#include "SettingsScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/save/SaveFile.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

extern "C" {
#include "player/fxes.h"
}

#define TITLE "SETTINGS"

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
}

void SettingsScene::processSelection() {
  if (selectButton->hasBeenPressedNow()) {
    fxes_play(SOUND_STEP);
    pixelBlink->blink();
  }

  if (nextButton->hasBeenPressedNow())
    move(1);

  if (backButton->hasBeenPressedNow())
    move(-1);
}

void SettingsScene::printMenu() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  BACKGROUND_write(TITLE, 2);

  printOption(0, "Audio lag", "1273", 5);
  printOption(1, "Show controls", "ON", 7);
  printOption(2, "Arrows' position", "LEFT", 9);
  printOption(3, "Background type", "HALF_DARK", 11);
  printOption(4, "Background blink", "ON", 13);
  printOption(5, "QUIT GAME", "", 15);
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
    selected = 0;
  else if (selected == OPTION_MIN && direction < 0)
    selected = OPTION_MAX;
  else
    selected += direction;

  fxes_play(SOUND_STEP);
  pixelBlink->blink();
  printMenu();
}
