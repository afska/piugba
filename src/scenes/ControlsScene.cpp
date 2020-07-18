#include "ControlsScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_controls.h"
#include "gameplay/Key.h"
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
const u32 RIGHT_CENTER = 5;
const u32 INSTRUCTOR_X = 86;
const u32 INSTRUCTOR_Y = 49;
const u32 START_COMBO_TOTAL = 5;
const ArrowDirection START_COMBO[] = {
    ArrowDirection::UPLEFT, ArrowDirection::UPRIGHT, ArrowDirection::UPLEFT,
    ArrowDirection::UPRIGHT, ArrowDirection::CENTER};

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

  sprites.push_back(instructor->get());

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(indicators[i]->get());
  sprites.push_back(indicators[RIGHT_CENTER]->get());

  for (u32 i = 0; i < START_COMBO_TOTAL; i++)
    sprites.push_back(inputArrows[i]->get());

  return sprites;
}

void ControlsScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);

  setUpSpritesPalette();
  setUpBackground();
  TextStream::instance().clear();
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  setUpArrows();
  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::Girl, INSTRUCTOR_X, INSTRUCTOR_Y)};
}

void ControlsScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    indicators[i]->tick();
  indicators[RIGHT_CENTER]->tick();
  for (u32 i = 0; i < START_COMBO_TOTAL; i++)
    inputArrows[i]->tick();

  processKeys(keys);

  // if (keys & KEY_ANY) {
  //   fxes_stop();
  //   engine->transitionIntoScene(new SelectionScene(engine, fs),
  //                               new FadeOutScene(2));
  // }
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

void ControlsScene::setUpArrows() {
  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    auto direction = static_cast<ArrowDirection>(i);
    indicators.push_back(
        std::unique_ptr<ArrowSelector>{new ArrowSelector(direction, true)});
  }

  indicators.push_back(std::unique_ptr<ArrowSelector>{new ArrowSelector(
      static_cast<ArrowDirection>(ArrowDirection::CENTER), true)});
  SPRITE_reuseTiles(indicators[RIGHT_CENTER]->get());

  indicators[ArrowDirection::DOWNLEFT]->get()->moveTo(22, 67);
  indicators[ArrowDirection::UPLEFT]->get()->moveTo(31, 59);
  indicators[ArrowDirection::CENTER]->get()->moveTo(39, 68);
  indicators[ArrowDirection::UPRIGHT]->get()->moveTo(187, 25);
  indicators[ArrowDirection::DOWNRIGHT]->get()->moveTo(199, 65);
  indicators[RIGHT_CENTER]->get()->moveTo(183, 74);

  for (u32 i = 0; i < START_COMBO_TOTAL; i++)
    inputArrows.push_back(
        std::unique_ptr<ArrowTutorial>{new ArrowTutorial(START_COMBO[i])});

  inputArrows[0]->get()->moveTo(76, 142);
  inputArrows[1]->get()->moveTo(94, 142);
  inputArrows[2]->get()->moveTo(112, 142);
  inputArrows[3]->get()->moveTo(130, 142);
  inputArrows[4]->get()->moveTo(148, 141);
}

void ControlsScene::processKeys(u16 keys) {
  indicators[ArrowDirection::DOWNLEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  indicators[ArrowDirection::UPLEFT]->setIsPressed(KEY_UPLEFT(keys));
  indicators[ArrowDirection::CENTER]->setIsPressed(KEY_CENTER(keys));
  indicators[ArrowDirection::UPRIGHT]->setIsPressed(KEY_UPRIGHT(keys));
  indicators[ArrowDirection::DOWNRIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
}
