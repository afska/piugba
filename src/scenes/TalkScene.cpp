#include "TalkScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "data/content/_compiled_sprites/palette_controls.h"
#include "gameplay/Key.h"
#include "utils/SceneUtils.h"
#include "utils/StringUtils.h"

#define LINE_BREAK "\r\n"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_BLEND_ALPHA = 12;
const int TEXT_OFFSET_X = 2;
const int TEXT_BASE_OFFSET_Y = -3;
const int TEXT_LINE_OFFSETS_Y[] = {0, -24, -16, -8, 0};

const u32 INSTRUCTOR_X = 2;
const u32 INSTRUCTOR_Y = 96;
const u32 BUTTON_MARGIN = 3;

TalkScene::TalkScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     std::string message,
                     std::function<void(u16 keys)> onKeyPress)
    : Scene(engine) {
  this->fs = fs;
  this->lines = STRING_split(message, LINE_BREAK);
  this->onKeyPress = onKeyPress;
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

  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);

  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::Boy, INSTRUCTOR_X, INSTRUCTOR_Y)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  if (!withButton)
    SPRITE_hide(nextButton->get());

  alignText();
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
  if ((keys & KEY_ANY) && hasFinished())
    onKeyPress(keys);

  nextButton->tick();

  autoWrite();
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

void TalkScene::alignText() {
  auto offsetY = TEXT_BASE_OFFSET_Y + TEXT_LINE_OFFSETS_Y[lines.size()];
  TextStream::instance().scroll(TEXT_OFFSET_X, offsetY);
  // 25 cols, 7 rows (separated by blanks, so actually 4 of them are usable)
}

void TalkScene::autoWrite() {
  wait = !wait;
  if (wait || hasFinished())
    return;

  auto character = lines[row].substr(col, 1);
  if (character != "") {
    TextStream::instance().setText(character, 2 + row * 2, col);
    col++;
  } else {
    row++;
    col = 0;
  }
}

TalkScene::~TalkScene() {
  lines.clear();
}
