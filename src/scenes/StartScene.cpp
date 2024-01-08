#include "StartScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
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

const std::string TITLES[] = {"Campaign",   "Arcade",     "Multi VS",
                              "Single",     "Multi COOP", "Challenges",
                              "Impossible", "DeathMix"};

const u32 BPM = 145;
const u32 ARROW_POOL_SIZE = 10;
const u32 DEMO_ARROW_INITIAL_Y = 78;
const u32 BUTTONS_TOTAL = 8;

const u32 ID_DARKENER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 20;
const u32 TEXT_COLOR = 0x6DEF;
const u32 TEXT_ROW = 12;
const int TEXT_OFFSET_Y = -4;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 ALPHA_BLINK_LEVEL = 8;
const u32 BUTTONS_X[] = {141, 175, 170, 183, 196, 209, 209, 222};
const u32 BUTTONS_Y[] = {128, 128, 136, 136, 136, 128, 136, 136};
const u32 BOUNCE_STEPS[] = {0, 6, 10, 13, 11, 9, 6, 3, 1, 0};
const u32 INPUT_LEFT = 0;
const u32 INPUT_RIGHT = 1;
const u32 INPUT_SELECT = 2;
const u32 INPUT_SELECT_ALT = 3;
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
  sprites.push_back(buttons[6]->get());
  sprites.push_back(buttons[7]->get());

  sprites.push_back(inputs[INPUT_LEFT]->get());
  sprites.push_back(inputs[INPUT_RIGHT]->get());
  sprites.push_back(inputs[INPUT_SELECT]->get());
  sprites.push_back(inputs[INPUT_SELECT_ALT]->get());

  arrowPool->forEach([&sprites](Arrow* it) {
    it->index = sprites.size();
    sprites.push_back(it->get());
  });

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void StartScene::load() {
  SAVEFILE_write8(SRAM->state.isPlaying, false);
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  pixelBlink = std::unique_ptr<PixelBlink>{new PixelBlink(PIXEL_BLINK_LEVEL)};

  setUpInputs();
  setUpButtons();
  setUpGameAnimation();

  TextStream::instance().scroll(0, TEXT_OFFSET_Y);
}

void StartScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    darkener->initialize(BackgroundType::FULL_BGA_DARK, 254);
    printTitle();
    BACKGROUND_enable(true, true, true, false);
    SPRITE_enable();
    hasStarted = true;
    player_loop(SOUND_LOOP);
  }

  __qran_seed += (1 + keys) * REG_VCOUNT;
  processKeys(keys);

  animateBpm();
  int bounceOffset = bounceDirection * BOUNCE_STEPS[darkenerOpacity] *
                     !!didWinImpossibleMode();
  darkenerOpacity = min(darkenerOpacity + 1, ALPHA_BLINK_LEVEL);
  EFFECT_setBlendAlpha(darkenerOpacity);

  animateInputs(bounceOffset);
  for (auto& it : inputs)
    it->tick();
  animateArrows(bounceOffset);

  processSelectionChange();
  navigateToAdminMenuIfNeeded(keys);

  pixelBlink->tick();
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

void StartScene::setUpInputs() {
  inputs.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNLEFT, false, true)});
  inputs.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::DOWNRIGHT, true, true)});
  inputs.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, true, true)});
  inputs.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, true, true)});
  inputs.push_back(std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, true, true)});

  animateInputs(0);
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
  buttons.push_back(std::unique_ptr<Button>{new Button(
      ButtonType::SUB_BUTTON_ORANGE, BUTTONS_X[6], BUTTONS_Y[6], true)});
  buttons.push_back(std::unique_ptr<Button>{new Button(
      ButtonType::SUB_BUTTON_ORANGE, BUTTONS_X[7], BUTTONS_Y[7], true)});

  buttons[2]->hide();
  buttons[3]->hide();
  buttons[4]->hide();
  buttons[6]->hide();
  buttons[7]->hide();

  if (ENV_ARCADE) {
    isArcadeExpanded = true;
    buttons[0]->hide();
    buttons[1]->hide();
    buttons[2]->show();
    buttons[3]->show();
    buttons[4]->show();
    buttons[5]->hide();
    buttons[6]->hide();
    buttons[7]->hide();
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
  int beat = MATH_fracumul(msecs * BPM, FRACUMUL_DIV_BY_MINUTE);
  int tick =
      MATH_fracumul(msecs * BPM * getTickCount(), FRACUMUL_DIV_BY_MINUTE);

  if (beat != lastBeat && beat != 0) {
    lastBeat = beat;
    darkenerOpacity = 0;
    bounceDirection *= -1;

    for (auto& it : arrowHolders)
      it->blink();
  }

  if (tick != lastTick && beat != 0) {
    lastTick = tick;

    arrowPool->create([this](Arrow* it) {
      it->initialize(ArrowType::UNIQUE,
                     static_cast<ArrowDirection>(qran_range(0, ARROWS_TOTAL)),
                     0, 0, didWinImpossibleMode() && qran_range(1, 101) > 50);
      it->get()->moveTo(it->get()->getX(), DEMO_ARROW_INITIAL_Y);
      it->press();
    });
  }
}

void StartScene::animateArrows(int bounceOffset) {
  for (auto& it : arrowHolders)
    it->tick();

  u32 arrowSpeed = getArrowSpeed();
  arrowPool->forEachActive([this, arrowSpeed, bounceOffset](Arrow* arrow) {
    int newY = arrow->get()->getY() - arrowSpeed;

    arrow->tick(newY, false, bounceOffset);
    if (arrow->needsDiscard())
      arrowPool->discard(arrow->id - ARROW_TILEMAP_LOADING_ID);
  });
}

void StartScene::animateInputs(int bounceOffset) {
  if (SAVEFILE_isUsingGBAStyle()) {
    inputs[INPUT_LEFT]->get()->moveTo(23 + bounceOffset, 48);
    inputs[INPUT_RIGHT]->get()->moveTo(42 + bounceOffset, 48);
    inputs[INPUT_SELECT]->get()->moveTo(202 + bounceOffset, 46);
    SPRITE_hide(inputs[INPUT_SELECT_ALT]->get());
  } else {
    inputs[INPUT_LEFT]->get()->moveTo(24 + bounceOffset, 47);
    inputs[INPUT_RIGHT]->get()->moveTo(201 + bounceOffset, 44);
    inputs[INPUT_SELECT]->get()->moveTo(42 + bounceOffset, 47);
    inputs[INPUT_SELECT_ALT]->get()->moveTo(185 + bounceOffset, 53);
  }
}

void StartScene::updateExpandedOrCollapsedButtons() {
  isArcadeExpanded = ENV_ARCADE || (selectedMode >= 2 && selectedMode <= 4);
  isChallengesExpanded = selectedMode >= 6;

  if (isArcadeExpanded) {
    buttons[1]->hide();
    buttons[2]->show();
    buttons[3]->show();
    buttons[4]->show();
  } else {
    buttons[1]->show();
    buttons[2]->hide();
    buttons[3]->hide();
    buttons[4]->hide();
  }

  if (isChallengesExpanded) {
    buttons[5]->hide();
    buttons[6]->show();
    buttons[7]->show();
  } else {
    buttons[5]->show();
    buttons[6]->hide();
    buttons[7]->hide();
  }
}

void StartScene::printTitle() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  SCENE_write(std::string((char*)gbfs_get_obj(fs, ROM_NAME_FILE, NULL)), 0);
  SCENE_write(TITLES[selectedMode], TEXT_ROW);
}

void StartScene::processKeys(u16 keys) {
  if (SAVEFILE_isUsingGBAStyle()) {
    inputs[INPUT_LEFT]->setIsPressed(keys & KEY_LEFT);
    inputs[INPUT_RIGHT]->setIsPressed(keys & KEY_RIGHT);
    inputs[INPUT_SELECT]->setIsPressed(keys & KEY_A);
    inputs[INPUT_SELECT_ALT]->setIsPressed(false);
  } else {
    inputs[INPUT_LEFT]->setIsPressed(KEY_DOWNLEFT(keys));
    inputs[INPUT_RIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
    inputs[INPUT_SELECT]->setIsPressed(KEY_CENTER(keys));
    inputs[INPUT_SELECT_ALT]->setIsPressed(KEY_CENTER(keys));
  }
}

void StartScene::processSelectionChange() {
  bool canGoLeft =
      (!ENV_ARCADE && selectedMode > 0) || (ENV_ARCADE && selectedMode > 2);
  if (inputs[INPUT_LEFT]->hasBeenPressedNow() && canGoLeft) {
    selectedMode--;

    if (selectedMode == 5 && isChallengesExpanded)
      selectedMode--;
    if (selectedMode == 4 && !isArcadeExpanded)
      selectedMode -= 3;
    if (selectedMode == 1 && isArcadeExpanded)
      selectedMode--;

    updateExpandedOrCollapsedButtons();
    pixelBlink->blink();
    printTitle();
  }

  bool canGoRight =
      (!ENV_ARCADE && ((isChallengesExpanded && selectedMode < 7) ||
                       (!isChallengesExpanded && selectedMode < 5))) ||
      (ENV_ARCADE && selectedMode < 4);
  if (inputs[INPUT_RIGHT]->hasBeenPressedNow() && canGoRight) {
    selectedMode++;

    if (selectedMode == 2 && !isArcadeExpanded)
      selectedMode += 3;
    if (selectedMode == 1 && isArcadeExpanded)
      selectedMode++;
    if (selectedMode == 5 && isChallengesExpanded)
      selectedMode++;

    updateExpandedOrCollapsedButtons();
    pixelBlink->blink();
    printTitle();
  }

  if (inputs[INPUT_SELECT]->hasBeenPressedNow())
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
  return (keys & KEY_L) && (keys & KEY_R) && (keys & KEY_START) &&
         (keys & KEY_SELECT);
}

void StartScene::goToGame() {
  if (selectedMode == 1) {
    selectedMode = 3;
    updateExpandedOrCollapsedButtons();
    pixelBlink->blink();
    printTitle();
    return;
  }
  if (selectedMode == 5) {
    bool isImpossibleModeUnlocked =
        SAVEFILE_isModeUnlocked(GameMode::IMPOSSIBLE);
    if (isImpossibleModeUnlocked) {
      isChallengesExpanded = true;
      selectedMode = 6;
      updateExpandedOrCollapsedButtons();
      pixelBlink->blink();
      printTitle();
      return;
    }
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
    case 5:
    case 6: {
      gameMode = GameMode::IMPOSSIBLE;
      break;
    }
    case 7: {
      gameMode = GameMode::DEATH_MIX;
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
  inputs.clear();
  arrowHolders.clear();
}
