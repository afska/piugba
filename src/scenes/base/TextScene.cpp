#include "TextScene.h"

#include <libgba-sprite-engine/background/text_stream.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_controls.h"
#include "gameplay/debug/DebugTools.h"
#include "utils/SceneUtils.h"
#include "utils/StringUtils.h"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_BLEND_ALPHA = 12;
const int TEXT_OFFSET_X = 2;
const int TEXT_BASE_OFFSET_Y = -3;
const int TEXT_LINE_OFFSETS_Y[] = {0, -24, -16, -8, 0};

TextScene::TextScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> TextScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> TextScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void TextScene::load() {
  SCENE_init();
  TextStream::instance().setMosaic(true);

  setUpSpritesPalette();
  setUpBackground();

  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);
}

void TextScene::tick(u16 keys) {
  if (engine->isTransitioning() || !hasStarted)
    return;

  autoWrite();
}

void TextScene::render() {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }
}

void TextScene::write(std::string text) {
  lines = STRING_split(text, LINE_BREAK);
  col = 0;
  row = 0;
  wait = true;
  alignText();
}

void TextScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>{new ForegroundPaletteManager(
          palette_controlsPal, sizeof(palette_controlsPal))};
}

void TextScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_TALK_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_TALK_TILES, BG_TALK_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void TextScene::alignText() {
  auto offsetY = TEXT_BASE_OFFSET_Y + TEXT_LINE_OFFSETS_Y[lines.size()];
  TextStream::instance().scrollNow(TEXT_OFFSET_X, offsetY);
  // 25 cols, 7 rows (separated by blanks, so actually 4 of them are usable)
}

void TextScene::autoWrite() {
  if (col == 0 && row == 0)
    TextStream::instance().clear();

  wait = !wait;
  if (wait || hasFinished())
    return;

  auto character = lines[row].substr(col, 1);
  if (character != "") {
    TextStream::instance().setText(character, 2 + row * 2, col);
    col++;
    if (character == " ")
      wait = true;
  } else {
    row++;
    col = 0;
  }
}

TextScene::~TextScene() {
  lines.clear();
}
