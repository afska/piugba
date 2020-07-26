#include "CalibrateScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "player/PlaybackState.h"
// #include "scenes/SelectionScene.h"
#include "gameplay/Key.h"
#include "utils/BackgroundUtils.h"
#include "utils/EffectUtils.h"

extern "C" {
#include "player/player.h"
}

#define TITLE "AUDIO LAG CALIBRATION"
#define SUBTITLE1 "1) Press to start"
#define SUBTITLE2 "2) Wait 4 beats"
#define SUBTITLE3 "3) Press on beat 5"
#define MEASURE_TITLE "- Detected audio lag -"

const u32 BPM = 120;
const u32 BEATS_TOTAL = 16;
const u32 TARGET_BEAT_MS = 2000;

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_ROW_TITLE = 3;
const u32 TEXT_ROW_SUBTITLE1 = 5;
const u32 TEXT_ROW_SUBTITLE2 = 6;
const u32 TEXT_ROW_SUBTITLE3 = 7;
const u32 TEXT_ROW_MEASURE_TITLE = 12;
const u32 TEXT_ROW_MEASURE_VALUE = 13;
const u32 TEXT_COL_SUBTITLE = 3;
const u32 TEXT_MIDDLE_COL = 12;
const u32 SELECTOR_X = 112;
const u32 SELECTOR_Y = 72;
const u32 PIXEL_BLINK_LEVEL = 6;

CalibrateScene::CalibrateScene(std::shared_ptr<GBAEngine> engine,
                               const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> CalibrateScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> CalibrateScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(selector->get());

  return sprites;
}

void CalibrateScene::load() {
  EFFECT_turnOffBlend();
  EFFECT_turnOffMosaic();
  BACKGROUND_enable(false, false, false, false);

  setUpSpritesPalette();
  setUpBackground();
  TextStream::instance().clear();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  selector = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::CENTER, false, true)};
  selector->get()->moveTo(SELECTOR_X, SELECTOR_Y);

  printTitle();
}

void CalibrateScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
  }

  selector->setIsPressed(KEY_CENTER(keys));

  if (selector->hasBeenPressedNow())
    calibrate();

  pixelBlink->tick();
  selector->tick();

  if (isMeasuring && PlaybackState.hasFinished)
    finish();

  // if (PlaybackState.hasFinished && (keys & KEY_ANY)) {
  //   player_stop();
  //   engine->transitionIntoScene(new SelectionScene(engine, fs),
  //                               new FadeOutScene(2));
  // }
}

void CalibrateScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal)));
}

void CalibrateScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_LINES_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_LINES_TILES, BG_LINES_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void CalibrateScene::printTitle() {
  TextStream::instance().setFontColor(TEXT_COLOR);
  TextStream::instance().clear();

  TextStream::instance().setText(
      TITLE, TEXT_ROW_TITLE, TEXT_MIDDLE_COL - std::string(TITLE).length() / 2);
  TextStream::instance().setText(SUBTITLE1, TEXT_ROW_SUBTITLE1,
                                 TEXT_COL_SUBTITLE);
  TextStream::instance().setText(SUBTITLE2, TEXT_ROW_SUBTITLE2,
                                 TEXT_COL_SUBTITLE);
  TextStream::instance().setText(SUBTITLE3, TEXT_ROW_SUBTITLE3,
                                 TEXT_COL_SUBTITLE);
}

void CalibrateScene::calibrate() {
  pixelBlink->blink();

  if (!isMeasuring) {
    start();
    return;
  }

  u32 msecs = PlaybackState.msecs;
  measuredLag = msecs - TARGET_BEAT_MS;
  finish();
}

void CalibrateScene::start() {
  isMeasuring = true;
  player_play(SOUND_CALIBRATE);
  printTitle();
}

void CalibrateScene::finish() {
  isMeasuring = false;

  TextStream::instance().setText(
      MEASURE_TITLE, TEXT_ROW_MEASURE_TITLE,
      TEXT_MIDDLE_COL - std::string(MEASURE_TITLE).length() / 2);

  auto value = std::to_string(measuredLag);
  TextStream::instance().setText(value, TEXT_ROW_MEASURE_VALUE,
                                 TEXT_MIDDLE_COL - value.length() / 2);
}
