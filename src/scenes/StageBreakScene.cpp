#include "StageBreakScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_break.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 INSTRUCTOR_X = 152;
const u32 INSTRUCTOR_Y = 48;

StageBreakScene::StageBreakScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> StageBreakScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> StageBreakScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(instructor->get());

  return sprites;
}

void StageBreakScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);
  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::AngryGirl, INSTRUCTOR_X, INSTRUCTOR_Y)};

  setUpSpritesPalette();
  setUpBackground();

  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();
  auto title = std::string("Hey! Why don't you...");
  TextStream::instance().setText(title, 1, 12 - title.length() / 2);
}

void StageBreakScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
    player_play(SOUND_STAGE_BREAK);
  }

  if (PlaybackState.hasFinished && keys & KEY_ANY) {
    player_stop();
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
  }
}

void StageBreakScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_breakPal, sizeof(palette_breakPal)));
}

void StageBreakScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_WALL_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_WALL_TILES, BG_WALL_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
}
