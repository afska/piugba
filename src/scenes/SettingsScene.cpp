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
#include "player/player.h"
}

#define TITLE "SETTINGS"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
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

  // sprites.push_back(calibrateButton->get());
  // sprites.push_back(resetButton->get());
  // sprites.push_back(saveButton->get());

  return sprites;
}

void SettingsScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);

  setUpSpritesPalette();
  setUpBackground();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

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

  pixelBlink->tick();
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
  // calibrateButton->setIsPressed(KEY_CENTER(keys));
  // resetButton->setIsPressed(KEY_UPLEFT(keys));
  // saveButton->setIsPressed(KEY_UPRIGHT(keys));
}

void SettingsScene::printMenu() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  BACKGROUND_write(TITLE, 2);

  TextStream::instance().setText("Audio lag", 5, -2);
  TextStream::instance().setText("<-1273>", 5, 18);

  TextStream::instance().setText(">Show controls", 7, -3);
  TextStream::instance().setText("<ON>", 7, 18);

  TextStream::instance().setText("Arrows' position", 9, -2);
  TextStream::instance().setText("<LEFT>", 9, 18);

  TextStream::instance().setText("Background type", 11, -2);
  TextStream::instance().setText("<HALF_DARK>", 11, 15);

  TextStream::instance().setText("Background blink", 13, -2);
  TextStream::instance().setText("<ON>", 13, 15);

  TextStream::instance().setText("EXIT", 15, -2);
}
