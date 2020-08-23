#include "MenuScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/fxes.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const int TEXT_COL_UNSELECTED = -2;
const int TEXT_COL_SELECTED = -3;
const int TEXT_COL_VALUE_MIDDLE = 20;
const u32 BUTTON_MARGIN = 8;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 SELECT_BUTTON_X = 112;

MenuScene::MenuScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> MenuScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> MenuScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(selectButton->get());
  sprites.push_back(backButton->get());
  sprites.push_back(nextButton->get());

  return sprites;
}

void MenuScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  selectButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  backButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNLEFT, true, true)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNRIGHT, true, true)};
  closeInput = std::unique_ptr<InputHandler>{new InputHandler()};

  selectButton->get()->moveTo(SELECT_BUTTON_X,
                              GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  backButton->get()->moveTo(BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  printMenu();
}

void MenuScene::tick(u16 keys) {
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

void MenuScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal)));
}

void MenuScene::setUpBackground() {
  loadBackground(ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void MenuScene::processKeys(u16 keys) {
  selectButton->setIsPressed(KEY_CENTER(keys));
  backButton->setIsPressed(KEY_DOWNLEFT(keys));
  nextButton->setIsPressed(KEY_DOWNRIGHT(keys));
  closeInput->setIsPressed(keys & getCloseKey());

  if (closeInput->hasBeenPressedNow()) {
    fxes_stop();
    engine->transitionIntoScene(new SelectionScene(engine, fs),
                                new FadeOutScene(2));
  }
}

void MenuScene::processSelection() {
  if (selectButton->hasBeenPressedNow())
    select();

  if (nextButton->hasBeenPressedNow())
    move(1);

  if (backButton->hasBeenPressedNow())
    move(-1);
}

void MenuScene::printMenu() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  printOptions();
}

void MenuScene::printOption(u32 id,
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

void MenuScene::move(int direction) {
  u32 maxOption = getOptionsCount() - 1;
  if (selected == maxOption && direction > 0)
    selected = 0;
  else if (selected == 0 && direction < 0)
    selected = maxOption;
  else
    selected += direction;

  fxes_play(SOUND_STEP);
  pixelBlink->blink();
  printMenu();
}

void MenuScene::select() {
  fxes_play(SOUND_STEP);
  pixelBlink->blink();

  if (selectOption(selected))
    printMenu();
  else
    fxes_stop();
}
