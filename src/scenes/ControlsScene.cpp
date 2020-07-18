#include "ControlsScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_controls.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

extern "C" {
#include "player/fxes.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 PIXEL_BLINK_LEVEL = 4;

ControlsScene::ControlsScene(std::shared_ptr<GBAEngine> engine,
                             const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> ControlsScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> ControlsScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void ControlsScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);

  // TODO: SETUP SPRITES

  setUpSpritesPalette();
  setUpBackground();
  TextStream::instance().clear();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));
}

void ControlsScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }

  if (keys & KEY_ANY) {
    fxes_stop();
    engine->transitionIntoScene(new SelectionScene(engine, fs),
                                new FadeOutScene(2));
  }
}

void ControlsScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_controlsPal, sizeof(palette_controlsPal)));
}

void ControlsScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_CONTROLS_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_CONTROLS_TILES, BG_CONTROLS_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}
