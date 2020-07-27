#include "TalkScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/content/_compiled_sprites/palette_controls.h"
#include "gameplay/Key.h"
#include "utils/SceneUtils.h"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_BLEND_ALPHA = 12;

const u32 INSTRUCTOR_X = 2;
const u32 INSTRUCTOR_Y = 96;
const u32 BUTTON_MARGIN = 3;

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
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  TextStream::instance().scroll(0, -4);
  // If the lines number is odd, MIDDLE_POINT = 9th row + 4px
  // If the lines number is even, MIDDLE_POINT = 9th & 10th rows
  // LINE_START = 0
  // MAX_LINE_COLS = 14
  // TODO: Center between 0~14 and vertically around MIDDLE_POINT

  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);

  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::Boy, INSTRUCTOR_X, INSTRUCTOR_Y)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  TextStream::instance().setText("Hello world!", 3, -1);
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
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_TALK_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_TALK_TILES, BG_TALK_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
}
