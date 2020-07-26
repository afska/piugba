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
  TextStream::instance().clear();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));
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
