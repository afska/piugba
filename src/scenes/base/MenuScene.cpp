#include "MenuScene.h"

#include <libgba-sprite-engine/background/text_stream.h>

#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const int TEXT_COL_UNSELECTED = -2;
const int TEXT_COL_SELECTED = -3;
const int TEXT_COL_VALUE_MIDDLE = 20;
const u32 BUTTON_MARGIN = 3;
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

  pixelBlink = std::unique_ptr<PixelBlink>{new PixelBlink(PIXEL_BLINK_LEVEL)};

  selectButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true, true, true)};
  backButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNLEFT, true, true, true, true)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNRIGHT, true, true, true, true)};
  closeInput = std::unique_ptr<InputHandler>{new InputHandler()};
  incrementInput = std::unique_ptr<InputHandler>{new InputHandler()};
  decrementInput = std::unique_ptr<InputHandler>{new InputHandler()};

  selectButton->get()->moveTo(SELECT_BUTTON_X,
                              GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  backButton->get()->moveTo(BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);
  nextButton->get()->moveTo(GBA_SCREEN_WIDTH - ARROW_SIZE - BUTTON_MARGIN,
                            GBA_SCREEN_HEIGHT - ARROW_SIZE - BUTTON_MARGIN);

  printMenu();
}

void MenuScene::tick(u16 keys) {
  if (engine->isTransitioning() || !hasStarted)
    return;

  processKeys(keys);
  processSelection();

  pixelBlink->tick();
  selectButton->tick();
  backButton->tick();
  nextButton->tick();
}

void MenuScene::render() {
  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }
}

void MenuScene::printOption(u32 id,
                            std::string name,
                            std::string value,
                            u32 row,
                            bool highlightChange,
                            std::string defaultValue) {
  bool isActive = selected == id;
  TextStream::instance().setText(
      std::string(isActive ? ">" : "") +
          (highlightChange && value != defaultValue ? "* " : "") + name,
      row, isActive ? TEXT_COL_SELECTED : TEXT_COL_UNSELECTED);

  if (value.length() > 0) {
    auto valueString = "<" + value + ">";
    TextStream::instance().setText(
        valueString, row, TEXT_COL_VALUE_MIDDLE - valueString.length() / 2);
  }
}

u8 MenuScene::change(u8 value, u8 optionsCount, int direction) {
  return direction >= 0 ? increment(value, optionsCount)
                        : decrement(value, optionsCount);
}

u8 MenuScene::increment(u8 value, u8 optionsCount) {
  return value >= optionsCount - 1 ? 0 : value + 1;
}

u8 MenuScene::decrement(u8 value, u8 optionsCount) {
  return value == 0 ? optionsCount - 1 : value - 1;
}

void MenuScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>{new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal))};
}

void MenuScene::setUpBackground() {
  loadBackground(ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void MenuScene::processKeys(u16 keys) {
  if (SAVEFILE_isUsingGBAStyle() != isUsingGBAStyle)
    blockButtons = true;

  if (blockButtons && !(keys & KEY_ANY))
    blockButtons = false;

  if (SAVEFILE_isUsingGBAStyle()) {
    isUsingGBAStyle = true;
    selectButton->setIsPressed(!blockButtons && (keys & KEY_A));
    backButton->setIsPressed(!blockButtons && (keys & KEY_UP));
    nextButton->setIsPressed(!blockButtons && (keys & KEY_DOWN));
    closeInput->setIsPressed(!blockButtons && (keys & getCloseKey()));
    incrementInput->setIsPressed(!blockButtons && (keys & (KEY_R | KEY_RIGHT)));
    decrementInput->setIsPressed(!blockButtons && (keys & (KEY_L | KEY_LEFT)));
  } else {
    isUsingGBAStyle = false;
    selectButton->setIsPressed(!blockButtons && (KEY_CENTER(keys)));
    backButton->setIsPressed(!blockButtons && (KEY_DOWNLEFT(keys)));
    nextButton->setIsPressed(!blockButtons && (KEY_DOWNRIGHT(keys)));
    closeInput->setIsPressed(!blockButtons && (keys & getCloseKey()));
    incrementInput->setIsPressed(!blockButtons && (KEY_UPRIGHT(keys)));
    decrementInput->setIsPressed(!blockButtons && (KEY_UPLEFT(keys)));
  }

  if (closeInput->hasBeenPressedNow())
    close();
}

void MenuScene::processSelection() {
  if (selectButton->hasBeenPressedNow())
    select(0);

  if (engine->isTransitioning())
    return;

  if (incrementInput->hasBeenPressedNow())
    select(1);

  if (decrementInput->hasBeenPressedNow())
    select(-1);

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

void MenuScene::move(int direction) {
  u32 maxOption = getOptionCount() - 1;
  if (selected == maxOption && direction > 0)
    selected = 0;
  else if (selected == 0 && direction < 0)
    selected = maxOption;
  else
    selected += direction;

  player_play(SOUND_STEP);
  pixelBlink->blink();
  printMenu();
}

void MenuScene::select(int direction) {
  player_play(SOUND_STEP);
  pixelBlink->blink();

  if (selectOption(selected, direction))
    printMenu();
  else
    player_stop();
}

void MenuScene::close() {
  player_stop();
  engine->transitionIntoScene(new SelectionScene(engine, fs),
                              new PixelTransitionEffect());
}
