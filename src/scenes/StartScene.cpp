#include "StartScene.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_start.h"
#include "gameplay/Key.h"
#include "gameplay/TimingProvider.h"
#include "gameplay/save/SaveFile.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/fxes.h"
#include "player/player.h"
}

const std::string TITLES[] = {"Campaign", "Arcade", "Impossible"};

const u32 BPM = 145;

const u32 ID_DARKENER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 20;
const u32 TEXT_COLOR = 0x6DEF;
const u32 TEXT_ROW = 12;
const int TEXT_OFFSET_Y = -4;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 ALPHA_BLINK_LEVEL = 8;
const u32 BUTTONS_X[] = {141, 175, 209};
const u32 BUTTONS_Y = 128;
const u32 INPUTS = 3;
const u32 INPUT_LEFT = 0;
const u32 INPUT_RIGHT = 1;
const u32 INPUT_SELECT = 2;
const u32 GAME_X = 72;

StartScene::StartScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;

  darkener = std::unique_ptr<Darkener>{new Darkener(ID_DARKENER, ID_DARKENER)};
  darkenerOpacity = ALPHA_BLINK_LEVEL;
}

std::vector<Background*> StartScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> StartScene::sprites() {
  std::vector<Sprite*> sprites;

  for (auto& it : buttons)
    sprites.push_back(it->get());

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void StartScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  setUpButtons();
  setUpGameAnimation();

  TextStream::instance().scroll(0, TEXT_OFFSET_Y);
}

void StartScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    darkener->initialize(0, BackgroundType::FULL_BGA_DARK, 254);
    printTitle();
    BACKGROUND_enable(true, true, true, false);
    SPRITE_enable();
    hasStarted = true;
    player_play(SOUND_LOOP);
  }

  if (PlaybackState.hasFinished)
    player_play(SOUND_LOOP);

  darkenerOpacity = min(darkenerOpacity + 1, ALPHA_BLINK_LEVEL);
  EFFECT_setBlendAlpha(darkenerOpacity);

  pixelBlink->tick();
  animateBpm();

  for (auto& it : arrowHolders)
    it->tick();

  processKeys(keys);
  processSelectionChange();
}

void StartScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_startPal, sizeof(palette_startPal)));
}

void StartScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_START_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_START_TILES, BG_START_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void StartScene::setUpButtons() {
  for (u32 i = 0; i < BUTTONS_TOTAL; i++) {
    auto type = static_cast<ButtonType>(i);
    buttons.push_back(std::unique_ptr<Button>{
        new Button(type, BUTTONS_X[i], BUTTONS_Y, type != ButtonType::BLUE)});
  }

  for (u32 i = 0; i < INPUTS; i++)
    inputHandlers.push_back(std::unique_ptr<InputHandler>{new InputHandler()});

  buttons[ButtonType::BLUE]->setSelected(true);
}

void StartScene::setUpGameAnimation() {
  STATE_reset();
  GameState.positionX = GAME_X;

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowDirection>(i), i > 0)});
  }
}

void StartScene::animateBpm() {
  int audioLag = (int)SAVEFILE_read32(SRAM->settings.audioLag);
  u32 msecs = PlaybackState.msecs - AUDIO_LAG - audioLag;
  u32 beat = Div(msecs * BPM, MINUTE);

  if (beat != lastBeat && beat != 0) {
    lastBeat = beat;
    darkenerOpacity = 0;
  }
}

void StartScene::printTitle() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  SCENE_write(TITLES[selectedMode], TEXT_ROW);
}

void StartScene::processKeys(u16 keys) {
  inputHandlers[INPUT_LEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  inputHandlers[INPUT_RIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
  inputHandlers[INPUT_SELECT]->setIsPressed(KEY_CENTER(keys));
}

void StartScene::processSelectionChange() {
  if (inputHandlers[INPUT_LEFT]->hasBeenPressedNow() && selectedMode > 0) {
    selectedMode--;
    pixelBlink->blink();
    printTitle();
  }

  if (inputHandlers[INPUT_RIGHT]->hasBeenPressedNow() &&
      selectedMode < BUTTONS_TOTAL - 1) {
    selectedMode++;
    pixelBlink->blink();
    printTitle();
  }

  if (inputHandlers[INPUT_RIGHT]->hasBeenPressedNow()) {
    // TODO: SELECT MODE
    // TODO: GO TO SELECTION SCREEN
  }

  for (u32 i = 0; i < BUTTONS_TOTAL; i++)
    buttons[i]->setSelected(selectedMode == i);
}

StartScene::~StartScene() {
  buttons.clear();
  inputHandlers.clear();
  arrowHolders.clear();
}
