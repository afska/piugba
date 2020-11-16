#include "StageBreakScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "SelectionScene.h"
#include "assets.h"
#include "data/content/_compiled_sprites/palette_break.h"
#include "gameplay/Sequence.h"
#include "gameplay/multiplayer/Syncer.h"
#include "player/PlaybackState.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR_START = 2686;
const u32 TEXT_COLOR_END = 0x7FFF;
const u32 TEXT_BLEND_ALPHA = 12;
const u32 INSTRUCTOR_X = 152;
const u32 INSTRUCTOR_Y = 48;
const u32 PIXEL_BLINK_LEVEL = 2;

#define WRITE(MSECS, TEXT, ROW, COL, DX, DY, FLIP_X, FLIP_Y)     \
  if (msecs > MSECS && step == totalSteps) {                     \
    TextStream::instance().setText(TEXT, ROW, COL);              \
    instructor->get()->moveTo(instructor->get()->getX() + (DX),  \
                              instructor->get()->getY() + (DY)); \
    isFlippedX = FLIP_X;                                         \
    isFlippedY = FLIP_Y;                                         \
    pixelBlink->blink();                                         \
    step++;                                                      \
  }                                                              \
  totalSteps++;

#define CLEAR(MSECS)                         \
  if (msecs > MSECS && step == totalSteps) { \
    TextStream::instance().clear();          \
    step++;                                  \
  }                                          \
  totalSteps++;

StageBreakScene::StageBreakScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> StageBreakScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> StageBreakScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(instructor->get());

  return sprites;
}

void StageBreakScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();

  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));
  EFFECT_setUpBlend(BLD_BG0, BLD_BG1);
  EFFECT_setBlendAlpha(TEXT_BLEND_ALPHA);

  instructor = std::unique_ptr<Instructor>{
      new Instructor(InstructorType::AngryGirl, INSTRUCTOR_X, INSTRUCTOR_Y)};
}

void StageBreakScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (SEQUENCE_isMultiplayerSessionDead()) {
    player_stop();
    SEQUENCE_goToMultiplayerGameMode(SAVEFILE_getGameMode());
    return;
  }

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
    player_play(SOUND_STAGE_BREAK);
  }

  if (isMultiplayer())
    processMultiplayerUpdates();

  animate();
  pixelBlink->tick();
  instructor->get()->flipHorizontally(isFlippedX);
  instructor->get()->flipVertically(isFlippedY);

  if (PlaybackState.hasFinished && (keys & KEY_ANY)) {
    if (isMultiplayer()) {
      if (syncer->isMaster())
        syncer->send(SYNC_EVENT_CONFIRM_SONG_END, 0);
      else
        return;
    }

    finish();
  }
}

void StageBreakScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_breakPal, sizeof(palette_breakPal)));
}

void StageBreakScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_WALL_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_WALL_TILES, BG_WALL_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void StageBreakScene::animate() {
  u32 msecs = PlaybackState.msecs;
  u32 totalSteps = 0;
  int heyRow = 4;
  int heyCol = 7;

  TextStream::instance().setFontColor(msecs >= 2110 ? TEXT_COLOR_END
                                                    : TEXT_COLOR_START);

  WRITE(1500, "H", heyRow, heyCol + 0, -1, 2, false, false);
  WRITE(1525, "e", heyRow, heyCol + 1, 3, -3, false, true);
  WRITE(1550, "e", heyRow, heyCol + 2, 1, 1, true, false);
  WRITE(1575, "e", heyRow, heyCol + 3, -3, -4, false, false);
  WRITE(1600, "e", heyRow, heyCol + 4, -6, 4, true, false);
  WRITE(1625, "e", heyRow, heyCol + 5, 2, -3, false, false);
  WRITE(1650, "y", heyRow, heyCol + 6, -2, -1, false, false);
  WRITE(1675, "!", heyRow, heyCol + 7, 1, 1, false, true);
  CLEAR(2110);

  WRITE(2116, "Why", /*   */ 6, 1, -10, -8, false, false);
  WRITE(2310, "don't", /* */ 7, 4, 2, 10, false, false);
  WRITE(2430, "you", /*   */ 8, 9, -4, -3, true, false);
  WRITE(2480, "just", /*  */ 9, 4, 6, -2, false, false);
  WRITE(2660, "get", /*   */ 10, 1, -3, -6, false, false);
  WRITE(2730, "up", /*    */ 11, 5, 4, 8, true, false);
  WRITE(2830, "and", /*   */ 12, 8, -2, 4, false, false);
  WRITE(2970, "dance,", /**/ 13, 1, -4, -4, false, false);
  WRITE(3280, "man?", /*  */ 14, 7, 2, 4, false, false);
}

void StageBreakScene::finish() {
  player_stop();
  engine->transitionIntoScene(new SelectionScene(engine, fs),
                              new FadeOutScene(2));
}

void StageBreakScene::processMultiplayerUpdates() {
  if (syncer->isMaster())
    return;

  auto linkState = linkConnection->linkState.get();
  auto remoteId = syncer->getRemotePlayerId();

  while (linkState->hasMessage(remoteId)) {
    u16 message = linkState->readMessage(remoteId);
    u8 event = SYNC_MSG_EVENT(message);

    switch (event) {
      case SYNC_EVENT_CONFIRM_SONG_END: {
        finish();

        break;
      }
      default: {
        syncer->registerTimeout();
      }
    }
  }
}
