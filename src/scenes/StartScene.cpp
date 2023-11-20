#include "StartScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include <string>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_start.h"
#include "gameplay/Key.h"
#include "gameplay/Sequence.h"
#include "gameplay/TimingProvider.h"
#include "gameplay/save/SaveFile.h"
#include "player/PlaybackState.h"
#include "scenes/AdminScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

const std::string TITLES[] = {"Campaign", "Arcade",     "Multi vs",
                              "Single",   "Multi coop", "Impossible"};

const u32 BPM = 145;
const u32 ARROW_POOL_SIZE = 30;
const u32 DEMO_ARROW_INITIAL_Y = 78;
const u32 BUTTONS_TOTAL = 6;

const u32 ID_DARKENER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 20;
const u32 TEXT_COLOR = 0x6DEF;
const u32 TEXT_ROW = 12;
const int TEXT_OFFSET_Y = -4;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 ALPHA_BLINK_LEVEL = 8;
const u32 BUTTONS_X[] = {141, 175, 170, 183, 196, 209};
const u32 BUTTONS_Y[] = {128, 128, 136, 136, 136, 128};
const u32 INPUTS = 3;
const u32 INPUT_LEFT = 0;
const u32 INPUT_RIGHT = 1;
const u32 INPUT_SELECT = 2;
const u32 GAME_X = 72;
const u32 GAME_Y = 13;

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

  sprites.push_back(buttons[0]->get());
  sprites.push_back(buttons[1]->get());
  sprites.push_back(buttons[5]->get());
  sprites.push_back(buttons[2]->get());
  sprites.push_back(buttons[3]->get());
  sprites.push_back(buttons[4]->get());

  arrowPool->forEach([&sprites](Arrow* it) {
    it->index = sprites.size();
    sprites.push_back(it->get());
  });

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void StartScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  pixelBlink = std::unique_ptr<PixelBlink>{new PixelBlink(PIXEL_BLINK_LEVEL)};

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
    player_loop(SOUND_LOOP);
  }

  darkenerOpacity = min(darkenerOpacity + 1, ALPHA_BLINK_LEVEL);
  EFFECT_setBlendAlpha(darkenerOpacity);

  pixelBlink->tick();
  animateBpm();
  animateArrows();

  __qran_seed += keys;
  processKeys(keys);
  processSelectionChange();
  navigateToAdminMenuIfNeeded(keys);
}

void StartScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>{
      new ForegroundPaletteManager(palette_startPal, sizeof(palette_startPal))};
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
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::BLUE, BUTTONS_X[0], BUTTONS_Y[0], false)});
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::GRAY, BUTTONS_X[1], BUTTONS_Y[1], true)});
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::SUB_BUTTON, BUTTONS_X[2], BUTTONS_Y[2], false)});
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::SUB_BUTTON, BUTTONS_X[3], BUTTONS_Y[3], true)});
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::SUB_BUTTON, BUTTONS_X[4], BUTTONS_Y[4], true)});
  buttons.push_back(std::unique_ptr<Button>{
      new Button(ButtonType::ORANGE, BUTTONS_X[5], BUTTONS_Y[5], true)});

  buttons[2]->hide();
  buttons[3]->hide();
  buttons[4]->hide();

  for (u32 i = 0; i < INPUTS; i++)
    inputHandlers.push_back(std::unique_ptr<InputHandler>{new InputHandler()});

  if (ENV_ARCADE) {
    isExpanded = true;
    buttons[0]->hide();
    buttons[1]->hide();
    buttons[2]->show();
    buttons[3]->show();
    buttons[4]->show();
    buttons[5]->hide();
    for (u32 i = 2; i <= 4; i++)
      buttons[i]->get()->moveTo(buttons[i]->get()->getX() + 26,
                                buttons[i]->get()->getY() + 2);
    selectedMode = 3;
    buttons[3]->setSelected(true);
  } else
    buttons[ButtonType::BLUE]->setSelected(true);
}

void StartScene::setUpGameAnimation() {
  STATE_setup();
  GameState.positionX[0] = GAME_X;
  GameState.positionY = GAME_Y;

  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{
      new ObjectPool<Arrow>(ARROW_POOL_SIZE, [](u32 id) -> Arrow* {
        return new Arrow(ARROW_TILEMAP_LOADING_ID + id);
      })};

  for (u32 i = 0; i < ARROWS_TOTAL; i++) {
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowDirection>(i), 0, true)});
    arrowHolders[i]->setIsPressed(false);
  }
}

void StartScene::animateBpm() {
  int audioLag = (int)GameState.settings.audioLag;
  int msecs = PlaybackState.msecs - audioLag;
  int beat = Div(msecs * BPM, MINUTE);
  int tick =
      MATH_fracumul(msecs * BPM * getTickCount(), FRACUMUL_DIV_BY_MINUTE);

  if (beat != lastBeat && beat != 0) {
    lastBeat = beat;
    darkenerOpacity = 0;

    pixelBlink->blink();
    for (auto& it : arrowHolders)
      it->blink();
  }

  if (tick != lastTick && beat != 0) {
    lastTick = tick;

    arrowPool->create([this](Arrow* it) {
      it->initialize(ArrowType::UNIQUE,
                     static_cast<ArrowDirection>(qran_range(0, ARROWS_TOTAL)),
                     0, 0);
      it->get()->moveTo(it->get()->getX(), DEMO_ARROW_INITIAL_Y);
      it->press();
    });
  }
}

void StartScene::animateArrows() {
  for (auto& it : arrowHolders)
    it->tick();

  u32 arrowSpeed = getArrowSpeed();
  arrowPool->forEachActive([this, arrowSpeed](Arrow* arrow) {
    int newY = arrow->get()->getY() - arrowSpeed;

    if (arrow->tick(newY, false) == ArrowState::OUT)
      arrowPool->discard(arrow->id - ARROW_TILEMAP_LOADING_ID);
  });
}

void StartScene::printTitle() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  SCENE_write(std::string((char*)gbfs_get_obj(fs, ROM_NAME_FILE, NULL)), 0);
  SCENE_write(TITLES[selectedMode], TEXT_ROW);
}

void StartScene::processKeys(u16 keys) {
  inputHandlers[INPUT_LEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  inputHandlers[INPUT_RIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
  inputHandlers[INPUT_SELECT]->setIsPressed(KEY_CENTER(keys));
}

void StartScene::processSelectionChange() {
  bool canGoLeft =
      (!ENV_ARCADE && selectedMode > 0) || (ENV_ARCADE && selectedMode > 2);
  if (inputHandlers[INPUT_LEFT]->hasBeenPressedNow() && canGoLeft) {
    selectedMode--;
    if (selectedMode == 4 && !isExpanded)
      selectedMode -= 3;
    if (selectedMode == 1 && isExpanded)
      selectedMode--;
    pixelBlink->blink();
    printTitle();
  }

  bool canGoRight =
      (!ENV_ARCADE && selectedMode < 5) || (ENV_ARCADE && selectedMode < 4);
  if (inputHandlers[INPUT_RIGHT]->hasBeenPressedNow() && canGoRight) {
    selectedMode++;
    if (selectedMode == 2 && !isExpanded)
      selectedMode += 3;
    if (selectedMode == 1 && isExpanded)
      selectedMode++;
    pixelBlink->blink();
    printTitle();
  }

  if (inputHandlers[INPUT_SELECT]->hasBeenPressedNow())
    goToGame();

  for (u32 i = 0; i < BUTTONS_TOTAL; i++)
    buttons[i]->setSelected(selectedMode == i);
}

void StartScene::navigateToAdminMenuIfNeeded(u16 keys) {
  if (!isPressingAdminCombo(keys))
    wasNotPressingAdminCombo = true;

  if (wasNotPressingAdminCombo && isPressingAdminCombo(keys)) {
    player_stop();
    engine->transitionIntoScene(new AdminScene(engine, fs),
                                new PixelTransitionEffect());
  }
}

bool StartScene::isPressingAdminCombo(u16 keys) {
  return KEY_DOWNLEFT(keys) && KEY_UPLEFT(keys) && KEY_UPRIGHT(keys) &&
         KEY_DOWNRIGHT(keys);
}

void StartScene::goToGame() {
  if (selectedMode == 1) {
    isExpanded = true;
    buttons[1]->hide();
    buttons[2]->show();
    buttons[3]->show();
    buttons[4]->show();
    selectedMode = 3;
    pixelBlink->blink();
    printTitle();
    return;
  }

  GameMode gameMode;
  switch (selectedMode) {
    case 2: {
      gameMode = GameMode::MULTI_VS;
      break;
    }
    case 3: {
      gameMode = GameMode::ARCADE;
      break;
    }
    case 4: {
      gameMode = GameMode::MULTI_COOP;
      break;
    }
    case 5: {
      gameMode = GameMode::IMPOSSIBLE;
      break;
    }
    default: {
      gameMode = GameMode::CAMPAIGN;
    }
  }

  player_stop();
  SEQUENCE_goToGameMode(gameMode);
}

StartScene::~StartScene() {
  buttons.clear();
  inputHandlers.clear();
  arrowHolders.clear();
}
