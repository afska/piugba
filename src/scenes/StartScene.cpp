#include "StartScene.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_start.h"
#include "gameplay/Key.h"
#include "gameplay/TimingProvider.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/EffectUtils.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/fxes.h"
#include "player/player.h"
}

const u32 BPM = 145;

const u32 ID_DARKENER = 1;
const u32 ID_MAIN_BACKGROUND = 2;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 20;
const u32 PIXEL_BLINK_LEVEL = 4;
const u32 ALPHA_BLINK_LEVEL = 10;
const u32 BUTTONS_X[] = {141, 175, 209};
const u32 BUTTONS_Y = 128;

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

  for (auto& button : buttons)
    sprites.push_back(button->get());

  return sprites;
}

void StartScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  setUpButtons();
  setUpGameAnimation();
}

void StartScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    darkener->initialize(0, BackgroundType::FULL_BGA_DARK);
    BACKGROUND_enable(true, true, true, false);
    SPRITE_enable();
    hasStarted = true;
    player_play(SOUND_LOOP);
  }

  processKeys(keys);

  if (PlaybackState.hasFinished)
    player_play(SOUND_LOOP);

  darkenerOpacity = min(darkenerOpacity + 1, ALPHA_BLINK_LEVEL);
  EFFECT_setBlendAlpha(darkenerOpacity);

  pixelBlink->tick();
  animateBpm();
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

  buttons[ButtonType::BLUE]->setSelected(true);
}

void StartScene::setUpGameAnimation() {
  // TODO: Implement
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

void StartScene::processKeys(u16 keys) {
  // buttons[ArrowDirection::DOWNLEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  // buttons[ArrowDirection::UPLEFT]->setIsPressed(KEY_UPLEFT(keys));
  // buttons[ArrowDirection::CENTER]->setIsPressed(KEY_CENTER(keys));
  // buttons[ArrowDirection::UPRIGHT]->setIsPressed(KEY_UPRIGHT(keys));
  // buttons[ArrowDirection::DOWNRIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
}
