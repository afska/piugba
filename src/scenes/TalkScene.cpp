#include "TalkScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/content/_compiled_sprites/palette_controls.h"
#include "gameplay/Key.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_BLEND_ALPHA = 12;
const u32 INSTRUCTOR_X = 152;
const u32 INSTRUCTOR_Y = 48;
const u32 NEXT_X = 220;
const u32 NEXT_Y = 140;

TalkScene::TalkScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     std::string message)
    : Scene(engine) {
  this->fs = fs;
  this->message = message;
}

std::vector<Background*> TalkScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> TalkScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(instructor->get());
  sprites.push_back(nextButton->get());

  return sprites;
}

void TalkScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);
  SPRITE_disable();

  setUpSpritesPalette();
  setUpBackground();
  TextStream::instance().clear();

  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);

  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::Boy, INSTRUCTOR_X, INSTRUCTOR_Y)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  nextButton->get()->moveTo(NEXT_X, NEXT_Y);

  TextStream::instance().setText("This is a test", 9, 0);
  TextStream::instance().setText("message. Hehe,", 10, 0);
  TextStream::instance().setText("too much words!", 11, 0);
}

void TalkScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }

  nextButton->setIsPressed(KEY_CENTER(keys));
  if (nextButton->hasBeenPressedNow()) {
    // TODO: Advance
  }

  nextButton->tick();
}

void TalkScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_controlsPal, sizeof(palette_controlsPal)));
}

void TalkScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_WALL_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_WALL_TILES, BG_WALL_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
}
