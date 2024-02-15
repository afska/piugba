#include "SongScene.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/palette/palette_manager.h>

#include "../libs/interrupt.h"
#include "DanceGradeScene.h"
#include "SelectionScene.h"
#include "StageBreakScene.h"
#include "data/content/_compiled_sprites/palette_song.h"
#include "gameplay/Key.h"
#include "gameplay/Sequence.h"
#include "gameplay/save/SaveFile.h"
#include "player/PlaybackState.h"
#include "scenes/ModsScene.h"
#include "ui/Darkener.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
#include "utils/flashio/everdrivex5/bios.h"
#include "utils/flashio/everdrivex5/disk.h"
}

#define DEBUG_OFFSET_CORRECTION 8

static u32 length;
static u8* video = NULL;
static u32 videoCursor = 0;
static u32 sdCursor = 0;

const u32 DARKENER_ID = 0;
const u32 DARKENER_PRIORITY = 2;
const u32 MAIN_BACKGROUND_ID = 1;
const u32 MAIN_BACKGROUND_PRIORITY = 3;
const u32 ARROW_POOL_SIZE = 90;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 24;
const u32 ALPHA_BLINK_LEVEL = 10;
const u32 PIXEL_BLINK_LEVEL = 2;
const u32 IO_BLINK_TIME = 6;
const u32 LIFEBAR_CHARBLOCK = 4;
const u32 LIFEBAR_TILE_START = 0;
const u32 LIFEBAR_TILE_END = 15;
const u32 RUMBLE_FRAMES = 4;
const u32 RUMBLE_PRELOAD_FRAMES = 2;
const u32 RUMBLE_IDLE_FREQUENCY = 5;
const u32 DEATH_MIX_ANTICIPATION_LEVEL = 6;
const u32 DEATH_MIX_SEEK_SPEED_FRAMES = 5;
const u32 BOUNCE_STEPS[] = {0, 1, 2, 4, 5,
                            8, 7, 5, 3, 0};  // <~>ALPHA_BLINK_LEVEL

static std::unique_ptr<Darkener> darkener{
    new Darkener(DARKENER_ID, DARKENER_PRIORITY)};

DATA_EWRAM static COLOR paletteBackups[TOTAL_COLOR_FILTERS]
                                      [PALETTE_MAX_SIZE * 2];
void backupPalettes(void (*onProgress)(u32 progress));
void reapplyFilter(ColorFilter colorFilter);

SongScene::SongScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     Song* song,
                     Chart* chart,
                     Chart* remoteChart,
                     std::unique_ptr<DeathMix> deathMix)
    : Scene(engine) {
  this->fs = fs;
  this->song = song;
  this->chart = chart;
  this->remoteChart = remoteChart != NULL ? remoteChart : chart;
  this->deathMix = std::move(deathMix);
}

std::vector<Background*> SongScene::backgrounds() {
#ifdef SENV_DEBUG
  return {};
#endif

  return {bg.get()};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(lifeBars[playerId]->get());

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(scores[playerId]->getFeedback()->get());
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(scores[playerId]->getCombo()->getTitle()->get());
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(scores[playerId]->getCombo()->getDigits()->at(0)->get());
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(scores[playerId]->getCombo()->getDigits()->at(1)->get());
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    sprites.push_back(scores[playerId]->getCombo()->getDigits()->at(2)->get());

  for (u32 i = 0; i < fakeHeads.size(); i++) {
    fakeHeads[i]->index = sprites.size();
    sprites.push_back(fakeHeads[i]->get());
  }

  arrowPool->forEach([&sprites](Arrow* it) {
    it->index = sprites.size();
    sprites.push_back(it->get());
  });

  for (auto& it : arrowHolders)
    sprites.push_back(it->get());

  return sprites;
}

void SongScene::load() {
  setUpGameConfig();
  RUMBLE_init();

  if (isMultiplayer()) {
    syncer->resetSongState();
    syncer->$isPlayingSong = true;
    syncer->$currentSongChecksum = song->id + chart->level + remoteChart->level;
    syncer->clearTimeout();
  }

  SCENE_init();

  setUpPalettes();
  setUpBackground();
  setUpArrows();

  pixelBlink = std::unique_ptr<PixelBlink>{new PixelBlink(PIXEL_BLINK_LEVEL)};

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    lifeBars[playerId] = std::unique_ptr<LifeBar>{new LifeBar(playerId)};

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    scores[playerId] =
        std::unique_ptr<Score>{new Score(lifeBars[playerId].get(), playerId)};

  judge = std::unique_ptr<Judge>{
      new Judge(arrowPool.get(), &arrowHolders, &scores,
                [this](u8 playerId) { onStageBreak(playerId); })};

  int audioLag = GameState.settings.audioLag;
  u32 multiplier =
      deathMix != NULL ? deathMix->multiplier : GameState.mods.multiplier;
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    chartReaders[playerId] = std::unique_ptr<ChartReader>{new ChartReader(
        playerId == localPlayerId ? chart : remoteChart, playerId,
        arrowPool.get(), judge.get(), pixelBlink.get(), audioLag, multiplier)};

  startInput = std::unique_ptr<InputHandler>{new InputHandler()};
  selectInput = std::unique_ptr<InputHandler>{new InputHandler()};
  aInput = std::unique_ptr<InputHandler>{new InputHandler()};
  bInput = std::unique_ptr<InputHandler>{new InputHandler()};
}

void SongScene::tick(u16 keys) {
  if (engine->isTransitioning() || init < 2)
    return;

  if (SEQUENCE_isMultiplayerSessionDead()) {
    unload();
    SEQUENCE_goToMultiplayerGameMode(SAVEFILE_getGameMode());
    return;
  }

  u32 songMsecs = PlaybackState.msecs;

  // if (PlaybackState.hasFinished || songMsecs >= song->lastMillisecond) {
  //   onStagePass();
  //   return;
  // }

  __qran_seed += (1 + keys) * REG_VCOUNT;
  processKeys(keys);

  if ($isMultiplayer) {
    processMultiplayerUpdates();
    if (!syncer->isPlaying())
      return;
  }

  drawVideo();

  bool isNewBeat = chartReaders[localPlayerId]->update((int)songMsecs);
  if (isNewBeat) {
    onNewBeat(KEY_ANY_PRESSED(keys));
    if (deathMix != NULL &&
        PlaybackState.msecs >= song->sampleStart + song->sampleLength) {
      finishAndGoToEvaluation();
      return;
    }
  }
  if ($isVs)
    chartReaders[syncer->getRemotePlayerId()]->update((int)songMsecs);

  updateBlink();
  updateArrowHolders();
  processModsTick();
  u8 minMosaic = processPixelateMod();
  pixelBlink->tick(minMosaic);

  updateFakeHeads();
  updateArrows();
  updateScoresAndLifebars();
  updateRumble();

#ifdef SENV_DEVELOPMENT
  if (chartReaders[0]->customOffset)
    scores[0]->log(chartReaders[0]->customOffset);

  IFTIMINGTEST {
    chartReaders[0]->logDebugInfo<CHART_DEBUG>();
  }
#endif
}

void SongScene::render() {
  if (engine->isTransitioning())
    return;

  darkener->render();

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    lifeBars[playerId]->tick(foregroundPalette.get());

  if (init == 0) {
    initializeBackground();
    init++;
    return;
  } else if (init == 1) {
    u16 keys = ~REG_KEYS & KEY_ANY;
    if (!initializeGame(keys))
      return;
    init++;
  }
}

void SongScene::setUpPalettes() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>{
      new ForegroundPaletteManager(palette_songPal, sizeof(palette_songPal))};

  backgroundPalette =
      BACKGROUND_loadPaletteFile(fs, song->backgroundPalettePath.c_str());
}

void SongScene::setUpBackground() {
#ifdef SENV_DEBUG
  return;
#endif

  bg = BACKGROUND_loadBackgroundFiles(fs, song->backgroundTilesPath.c_str(),
                                      song->backgroundMapPath.c_str(),
                                      MAIN_BACKGROUND_ID);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->usePriority(MAIN_BACKGROUND_PRIORITY);
  bg->setMosaic(true);

  // TODO: Remove test code
  bi_init();
  diskInit();
  videoCursor = 1024;
  sdCursor = 2098048;
}

void SongScene::setUpArrows() {
  arrowPool = std::unique_ptr<ObjectPool<Arrow>>{new ObjectPool<Arrow>(
      ARROW_POOL_SIZE, [](u32 id) -> Arrow* { return new Arrow(id); })};

  for (u32 i = 0; i < ARROWS_TOTAL * platformCount; i++) {
    auto direction = getDirectionFromIndex(i);
    auto arrowHolder = std::unique_ptr<ArrowHolder>{
        new ArrowHolder(direction, getPlayerIdFromIndex(i), true)};
    arrowHolder->get()->setPriority(ARROW_LAYER_BACK);
    arrowHolders.push_back(std::move(arrowHolder));

    auto fakeHead =
        std::unique_ptr<Arrow>{new Arrow(ARROW_TILEMAP_LOADING_ID + i)};
    SPRITE_hide(fakeHead->get());

    fakeHeads.push_back(std::move(fakeHead));
  }
}

void SongScene::initializeBackground() {
#ifdef SENV_DEBUG
  BACKGROUND_setColor(0, 127);
  return;
#endif

  darkener->initialize(GameState.settings.backgroundType);
  updateGameX();

  if (GameState.mods.colorFilter != ColorFilter::NO_FILTER) {
    SCENE_applyColorFilter(backgroundPalette.get(), GameState.mods.colorFilter);
    SCENE_applyColorFilter(foregroundPalette.get(), GameState.mods.colorFilter);
  }

  if (!$isVs)
    for (u32 i = LIFEBAR_TILE_START; i <= LIFEBAR_TILE_END; i++)
      BACKGROUND_createSolidTile(LIFEBAR_CHARBLOCK, i, 0);
}

bool SongScene::initializeGame(u16 keys) {
  if (deathMix != NULL && deathMix->didStartScroll)
    goto initialized;

  if (GameState.mods.autoMod) {
    EFFECT_setMosaic(MAX_MOSAIC);
    EFFECT_render();
  }
  BACKGROUND_enable(true, !ENV_DEBUG, false, false);
  SPRITE_enable();
  if (GameState.mods.autoMod)
    backupPalettes([](u32 progress) {
      EFFECT_setMosaic(max(MAX_MOSAIC - progress, 0));
      EFFECT_render();
    });

initialized:
  if (deathMix != NULL) {
    if (!deathMix->didStartScroll) {
      if (!deathMix->isInitialSong()) {
        scores[0]->setLife(deathMix->life);
        scores[0]->getCombo()->setValue(deathMix->combo);
        scores[0]->setHasMissCombo(deathMix->hasMissCombo);
        scores[0]->setHalfLifeBonus(deathMix->halfLifeBonus);
        scores[0]->setMaxCombo(deathMix->maxCombo);
        scores[0]->setCounters(deathMix->counters);
        scores[0]->setPoints(deathMix->points);
        scores[0]->setLongNotes(deathMix->longNotes);
        lifeBars[0]->tick(foregroundPalette.get());
      }

      arrowPool->turnOff();
      chartReaders[0]->turnOffObjectPools();

      GameState.mods.speedHack = SpeedHackOpts::hAUTO_VELOCITY;
      chartReaders[0]->setMultiplier(DEATH_MIX_ANTICIPATION_LEVEL);
      for (u32 t = 0; t < song->sampleStart;
           t += FRAME_MS * DEATH_MIX_SEEK_SPEED_FRAMES)
        chartReaders[0]->update(t);
      chartReaders[0]->update(song->sampleStart);
      GameState.mods.speedHack = SpeedHackOpts::hOFF;

      deathMix->didStartScroll = true;
      return false;
    } else {
      arrowPool->turnOn();
      chartReaders[0]->turnOnObjectPools();
      chartReaders[0]->setMultiplier(deathMix->multiplier);
    }
  }

  player_play(song->audioPath.c_str());
  if (deathMix != NULL)
    player_seek(song->sampleStart);
  processModsLoad();

  if (IS_ARCADE(SAVEFILE_getGameMode()) && (keys & KEY_START) &&
      (keys & KEY_SELECT)) {
    // (if START and SELECT are pressed on start, the chart will be marked as
    // defective and return to the selection scene)
    SAVEFILE_setGradeOf(song->index, chart->difficulty, song->id,
                        chart->levelIndex, GradeType::DEFECTIVE);

    if ($isMultiplayer) {
      syncer->send(SYNC_EVENT_ABORT, 0);
      syncer->clearTimeout();
    }

    onAbort();

    return false;
  }

  return true;
}

void SongScene::updateArrowHolders() {
  int bounceOffset = -bounceDirection * BOUNCE_STEPS[blinkFrame] *
                     (GameState.mods.bounce == BounceOpts::bALL);

  for (auto& it : arrowHolders)
    it->tick(bounceOffset);
}

CODE_IWRAM void SongScene::updateArrows() {
  std::array<Arrow*, ARROWS_TOTAL * GAME_MAX_PLAYERS> nextArrows;
  for (u32 i = 0; i < ARROWS_TOTAL * GAME_MAX_PLAYERS; i++)
    nextArrows[i] = NULL;

  int bounceOffset =
      bounceDirection * BOUNCE_STEPS[blinkFrame] * !!GameState.mods.bounce;

  // trackers
  int judgementOffset[GAME_MAX_PLAYERS];
  bool isStopped[GAME_MAX_PLAYERS];
  bool isOnStopEdge[GAME_MAX_PLAYERS];
  bool isStopAsync[GAME_MAX_PLAYERS];
  int stopStart[GAME_MAX_PLAYERS];
  u32 baseIndex[GAME_MAX_PLAYERS];
  for (u32 playerId = 0; playerId < playerCount; playerId++) {
    judgementOffset[playerId] = chartReaders[playerId]->getJudgementOffset();
    isStopped[playerId] = chartReaders[playerId]->isStopped();
    if (isStopped[playerId]) {
      isOnStopEdge[playerId] = chartReaders[playerId]->hasJustStopped() ||
                               chartReaders[playerId]->isAboutToResume();
      isStopAsync[playerId] = chartReaders[playerId]->isStopAsync();
      stopStart[playerId] = chartReaders[playerId]->getStopStart();
    }
    baseIndex[playerId] = getBaseIndexFromPlayerId(playerId);
  }

  // update sprites
  arrowPool->forEachActive([&nextArrows, &baseIndex, &bounceOffset, &isStopped,
                            &judgementOffset, this](Arrow* arrow) {
    ArrowDirection direction = arrow->direction;
    u32 playerId = arrow->playerId;
    u32 arrowBaseIndex = baseIndex[playerId];

    int newY = chartReaders[playerId]->getYFor(arrow);
    bool isPressing =
        arrowHolders[arrowBaseIndex + direction]->getIsPressed() &&
        !isStopped[playerId];
    bool isEnding = arrow->tick(newY, isPressing, bounceOffset);

    if (isEnding && !isStopped[playerId] &&
        judge->endIfNeeded(arrow, chartReaders[playerId].get())) {
      if (arrow->needsDiscard()) {
        arrow->forAll(arrowPool.get(),
                      [this](Arrow* arrow) { arrowPool->discard(arrow->id); });
      }
      return;
    }

    bool canBeJudged = arrow->type == ArrowType::UNIQUE &&
                       !arrow->getIsPressed() && !arrow->isFake;
    u32 index = arrowBaseIndex + direction;
    if (canBeJudged && (nextArrows[index] == NULL ||
                        arrow->timestamp < nextArrows[index]->timestamp))
      nextArrows[index] = arrow;
  });

  // judge key press events
  for (u32 i = 0; i < ARROWS_TOTAL * platformCount; i++) {
    auto arrow = nextArrows[i];
    if (arrow == NULL)
      continue;

    u32 playerId = arrow->playerId;
    ArrowDirection direction = arrow->direction;
    bool canBeJudged =
        !isStopped[playerId] || isStopAsync[playerId] ||
        (arrow->timestamp >= stopStart[playerId] && isOnStopEdge[playerId]);
    bool hasBeenPressedNow =
        arrowHolders[baseIndex[playerId] + direction]->hasBeenPressedNow();

    if (canBeJudged && hasBeenPressedNow) {
      auto isHit = judge->onPress(arrow, chartReaders[playerId].get(),
                                  judgementOffset[playerId]);

      if (playerId == localPlayerId && isHit &&
          GameState.adminSettings.sramBlink == SRAMBlinkOpts::SRAM_BLINK_ON_HIT)
        SAVEFILE_write8(SRAM->beat, 0);
    }
  }
}

void SongScene::updateBlink() {
  blinkFrame = max(blinkFrame - 1, 0);

  if ($isMultiplayer || GameState.settings.bgaDarkBlink)
    EFFECT_setBlendAlpha(ALPHA_BLINK_LEVEL - blinkFrame);

  if (!$isMultiplayer &&
      GameState.adminSettings.ioBlink == IOBlinkOpts::IO_BLINK_ON_BEAT) {
    if (blinkFrame >= ALPHA_BLINK_LEVEL - IO_BLINK_TIME)
      IOPORT_sdHigh();
    else
      IOPORT_sdLow();
  }
}

void SongScene::updateFakeHeads() {
  for (u32 i = 0; i < fakeHeads.size(); i++) {
    auto direction = getDirectionFromIndex(i);
    u8 playerId = getPlayerIdFromIndex(i);
    u8 baseIndex = getBaseIndexFromPlayerId(playerId);

    bool isHoldMode = chartReaders[playerId]->isHoldActive(direction);
    bool isPressing = arrowHolders[baseIndex + direction]->getIsPressed();
    bool isVisible = fakeHeads[i]->get()->enabled;

    if (isHoldMode && isPressing && !chartReaders[playerId]->isStopped()) {
      if (!isVisible) {
        fakeHeads[i]->initialize(ArrowType::HOLD_FAKE_HEAD, direction, playerId,
                                 0);
        isVisible = true;
      }
    } else if (isVisible) {
      fakeHeads[i]->discard();
      isVisible = false;
    }

    if (isVisible)
      fakeHeads[i]->tick(0, false);
  }
}

void SongScene::updateScoresAndLifebars() {
  for (u32 playerId = 0; playerId < playerCount; playerId++)
    scores[playerId]->tick();
  if ($isMultiplayer && $isVs)
    animateWinnerLifeBar();
}

void SongScene::updateGameX() {
  lifeBars[0]->relocate();
  scores[0]->relocate();

  auto backgroundType = GameState.settings.backgroundType;
  if (backgroundType == BackgroundType::HALF_BGA_DARK)
    darkener->setX(-GameState.positionX[0]);
}

void SongScene::updateGameY() {
  lifeBars[0]->get()->moveTo(lifeBars[0]->get()->getX(),
                             GameState.positionY + LIFEBAR_POSITION_Y);
  for (auto& it : arrowHolders)
    it->get()->moveTo(it->get()->getX(), ARROW_FINAL_Y());
}

void SongScene::updateRumble() {
  if (!GameState.adminSettings.rumble)
    return;

  auto localChartReader = chartReaders[localPlayerId].get();

  if (localChartReader->beatDurationFrames != -1 &&
      localChartReader->beatFrame ==
          localChartReader->beatDurationFrames - RUMBLE_PRELOAD_FRAMES) {
    rumbleIdleFrame = 0;
    rumbleBeatFrame = 0;
    RUMBLE_start();
  }

  if (rumbleBeatFrame > -1) {
    rumbleBeatFrame++;

    if (rumbleBeatFrame == RUMBLE_FRAMES) {
      rumbleBeatFrame = -1;
      RUMBLE_stop();
    }
  } else {
    rumbleIdleFrame++;

    if (rumbleIdleFrame == 1)
      RUMBLE_stop();
    else if (rumbleIdleFrame == RUMBLE_IDLE_FREQUENCY) {
      RUMBLE_start();
      rumbleIdleFrame = 0;
    }
  }
}

void SongScene::animateWinnerLifeBar() {
  u32 score0 =
      $isVsDifferentLevels ? scores[0]->getPercent() : scores[0]->getPoints();
  u32 score1 =
      $isVsDifferentLevels ? scores[1]->getPercent() : scores[1]->getPoints();
  bool isWinning0 = score0 > score1;
  bool isWinning1 = score1 > score0;

  lifeBars[0]->get()->moveTo(lifeBars[0]->get()->getX(),
                             GameState.positionY + LIFEBAR_POSITION_Y -
                                 BOUNCE_STEPS[blinkFrame * isWinning0] / 2);
  lifeBars[1]->get()->moveTo(lifeBars[1]->get()->getX(),
                             GameState.positionY + LIFEBAR_POSITION_Y -
                                 BOUNCE_STEPS[blinkFrame * isWinning1] / 2);
}

static bool videoinit = false;
static bool renderr = true;

void SongScene::drawVideo() {
  // TODO: Test code

  // 30fps
  if (!renderr) {
    renderr = true;
    return;
  } else {
    renderr = false;
  }

  if (!videoinit) {
    bi_init();
    diskInit();
    videoinit = true;
  }

  u8 buff[512];
  u8 resp;

  auto c1 = pal_bg_mem[254];
  auto c2 = pal_bg_mem[255];
  diskRead(sdCursor + videoCursor / 512, (u8*)pal_bg_mem, 1);
  videoCursor += 512;
  pal_bg_mem[254] = c1;
  pal_bg_mem[255] = c2;

  u32 backgroundTilesLength = 38912;
  u32 backgroundMapLength = 2048;

  Background background(MAIN_BACKGROUND_ID, NULL, backgroundTilesLength, NULL,
                        backgroundMapLength);
  background.useCharBlock(BANK_BACKGROUND_TILES);
  background.useMapScreenBlock(BANK_BACKGROUND_MAP);
  background.usePriority(MAIN_BACKGROUND_PRIORITY);
  background.setMosaic(true);
  background.persistNow([](void* dst, u32 size) {
    diskRead(sdCursor + videoCursor / 512, (u8*)dst, size / 512);
    videoCursor += size;
  });
}

void SongScene::processKeys(u16 keys) {
  u32 downLeftKeys = (keys & KEY_DOWN) | (keys & KEY_LEFT);
  u32 upLeftKeys = (keys & KEY_L) | (keys & KEY_UP);
  u32 centerKeys = (keys & KEY_B) | (keys & KEY_RIGHT);

  if (KEY_DOWNLEFT(keys) && downLeftKeys != lastDownLeftKeys)
    arrowHolders[localBaseIndex + 0]->setIsPressed(false);
  if (KEY_UPLEFT(keys) && upLeftKeys != lastUpLeftKeys)
    arrowHolders[localBaseIndex + 1]->setIsPressed(false);
  if (KEY_CENTER(keys) && centerKeys != lastCenterKeys)
    arrowHolders[localBaseIndex + 2]->setIsPressed(false);
  arrowHolders[localBaseIndex + 0]->setIsPressed(KEY_DOWNLEFT(keys));
  arrowHolders[localBaseIndex + 1]->setIsPressed(KEY_UPLEFT(keys));
  arrowHolders[localBaseIndex + 2]->setIsPressed(KEY_CENTER(keys));
  arrowHolders[localBaseIndex + 3]->setIsPressed(KEY_UPRIGHT(keys));
  arrowHolders[localBaseIndex + 4]->setIsPressed(KEY_DOWNRIGHT(keys));

  if ($isSinglePlayerDouble) {
    if (KEY_DOWNLEFT(keys) && downLeftKeys != lastDownLeftKeys)
      arrowHolders[localBaseIndex + 5]->setIsPressed(false);
    if (KEY_UPLEFT(keys) && upLeftKeys != lastUpLeftKeys)
      arrowHolders[localBaseIndex + 6]->setIsPressed(false);
    if (KEY_CENTER(keys) && centerKeys != lastCenterKeys)
      arrowHolders[localBaseIndex + 7]->setIsPressed(false);
    arrowHolders[localBaseIndex + 5]->setIsPressed(KEY_DOWNLEFT(keys));
    arrowHolders[localBaseIndex + 6]->setIsPressed(KEY_UPLEFT(keys));
    arrowHolders[localBaseIndex + 7]->setIsPressed(KEY_CENTER(keys));
    arrowHolders[localBaseIndex + 8]->setIsPressed(KEY_UPRIGHT(keys));
    arrowHolders[localBaseIndex + 9]->setIsPressed(KEY_DOWNRIGHT(keys));
  }

  lastDownLeftKeys = downLeftKeys;
  lastUpLeftKeys = upLeftKeys;
  lastCenterKeys = centerKeys;

  startInput->setIsPressed(keys & KEY_START);
  selectInput->setIsPressed(keys & KEY_SELECT);
  aInput->setIsPressed(keys & KEY_A);
  bInput->setIsPressed(keys & KEY_B);

  IFSTRESSTEST {
    for (auto& arrowHolder : arrowHolders)
      if (arrowHolder->getIsPressed())
        arrowPool->create([&arrowHolder, this](Arrow* it) {
          it->initialize(
              ArrowType::UNIQUE, arrowHolder->direction, 0,
              chartReaders[0]->getMsecs() + chartReaders[0]->getArrowTime());
        });
  }

  if (!$isMultiplayer &&
      GameState.adminSettings.ioBlink == IOBlinkOpts::IO_BLINK_ON_KEY) {
    if (KEY_ANY_PRESSED(keys))
      IOPORT_sdHigh();
    else
      IOPORT_sdLow();
  }

  if (GameState.mods.trainingMode != TrainingModeOpts::tOFF) {
    processTrainingModeMod();
    return;
  }

  if (startInput->hasBeenPressedNow()) {
    if (KEY_CENTER(keys) && ENV_DEVELOPMENT) {
      chartReaders[0]->customOffset -= DEBUG_OFFSET_CORRECTION;
      if (chartReaders[0]->customOffset == 0)
        scores[0]->log(0);
    } else if (GameState.mods.speedHack != SpeedHackOpts::hRANDOM) {
      if (chartReaders[localPlayerId]->setMultiplier(
              chartReaders[localPlayerId]->getMultiplier() + 1))
        pixelBlink->blink();

      if ($isMultiplayer)
        syncer->send(SYNC_EVENT_MULTIPLIER_CHANGE,
                     chartReaders[localPlayerId]->getMultiplier());
    }
  }

  if (selectInput->hasBeenPressedNow()) {
    if (KEY_CENTER(keys) && ENV_DEVELOPMENT) {
      chartReaders[0]->customOffset += DEBUG_OFFSET_CORRECTION;
      if (chartReaders[0]->customOffset == 0)
        scores[0]->log(0);
    } else if (GameState.mods.speedHack != SpeedHackOpts::hRANDOM) {
      if (chartReaders[localPlayerId]->setMultiplier(
              chartReaders[localPlayerId]->getMultiplier() - 1))
        pixelBlink->blink();

      if ($isMultiplayer)
        syncer->send(SYNC_EVENT_MULTIPLIER_CHANGE,
                     chartReaders[localPlayerId]->getMultiplier());
    }
  }
}

void SongScene::onNewBeat(bool isAnyKeyPressed) {
  blinkFrame = min(blinkFrame + ALPHA_BLINK_LEVEL, ALPHA_BLINK_LEVEL);

  for (u32 playerId = 0; playerId < playerCount; playerId++)
    lifeBars[playerId]->blink();

  for (auto& arrowHolder : arrowHolders)
    if (!isAnyKeyPressed)
      arrowHolder->blink();

  processModsBeat();

  auto localChartReader = chartReaders[localPlayerId].get();
  localChartReader->beatDurationFrames = localChartReader->beatFrame;
  localChartReader->beatFrame = 0;

  if (GameState.adminSettings.sramBlink == SRAMBlinkOpts::SRAM_BLINK_ON_BEAT)
    SAVEFILE_write8(SRAM->beat, 0);
}

void SongScene::onStageBreak(u8 playerId) {
  scores[playerId]->die();

  if ($isMultiplayer) {
    if ($isVs) {
      if (playerId == localPlayerId)
        syncer->send(SYNC_EVENT_STAGE_END, false);

      bool allDead = !ENV_DEVELOPMENT && lifeBars[localPlayerId]->getIsDead() &&
                     lifeBars[syncer->getRemotePlayerId()]->getIsDead();

      if (allDead)
        breakStage();
    } else {
      syncer->send(SYNC_EVENT_STAGE_END, false);

#ifdef SENV_DEVELOPMENT
      return;
#endif

      breakStage();
    }
  } else if (GameState.mods.stageBreak != StageBreakOpts::sOFF)
    breakStage();
}

void SongScene::onStagePass() {
  if ($isMultiplayer)
    syncer->send(SYNC_EVENT_STAGE_END, true);

  finishAndGoToEvaluation();
}

void SongScene::onAbort() {
  unload();
  engine->transitionIntoScene(new SelectionScene(engine, fs),
                              new PixelTransitionEffect());
}

void SongScene::breakStage() {
  unload();
  engine->transitionIntoScene(new StageBreakScene(engine, fs),
                              new PixelTransitionEffect());
}

void SongScene::finishAndGoToEvaluation() {
  unload();

  if (deathMix != NULL) {
    continueDeathMix();
    return;
  }

  auto evaluation = scores[localPlayerId]->evaluate();
  bool isLastSong =
      SAVEFILE_setGradeOf(song->index, chart->difficulty, song->id,
                          chart->levelIndex, evaluation->getGrade());

  engine->transitionIntoScene(
      new DanceGradeScene(
          engine, fs, std::move(evaluation),
          $isVs ? scores[syncer->getRemotePlayerId()]->evaluate() : NULL,
          $isVsDifferentLevels, isLastSong),
      new PixelTransitionEffect());
}

void SongScene::continueDeathMix() {
  int lastIndex =
      SAVEFILE_read8(SRAM->deathMixProgress.completedSongs[chart->difficulty]) -
      1;
  u8 librarySize = SAVEFILE_getLibrarySize();
  bool firstTime = (int)song->index > lastIndex;
  if (firstTime) {
    auto completedSongs = (u8)min(song->index + 1, librarySize);
    SAVEFILE_write8(SRAM->deathMixProgress.completedSongs[chart->difficulty],
                    completedSongs);
  }

  auto songChart = deathMix->getNextSongChart();

  if (songChart.song != NULL) {
    deathMix->didStartScroll = false;
    deathMix->multiplier = chartReaders[0]->getMultiplier();
    deathMix->life = scores[0]->getLife();
    deathMix->combo = scores[0]->getCombo()->getValue() *
                      (scores[0]->getHasMissCombo() ? -1 : 1);
    deathMix->hasMissCombo = scores[0]->getHasMissCombo();
    deathMix->halfLifeBonus = scores[0]->getHalfLifeBonus();
    deathMix->maxCombo = scores[0]->getMaxCombo();
    deathMix->counters = scores[0]->getCounters();
    deathMix->points = scores[0]->getPoints();
    deathMix->longNotes = scores[0]->getLongNotes();

    scores[0]->getCombo()->setValue(deathMix->getCurrentSongNumber());
    scores[0]->getCombo()->show();
    SPRITE_hide(scores[0]->getFeedback()->get());
    SPRITE_hide(scores[0]->getCombo()->getTitle()->get());

#ifdef SENV_DEVELOPMENT
    auto stageBreak = GameState.mods.stageBreak;
#endif
    STATE_setup(songChart.song, songChart.chart);
#ifdef SENV_DEVELOPMENT
    GameState.mods.stageBreak = stageBreak;
#endif

    engine->transitionIntoScene(
        new SongScene(engine, fs, songChart.song, songChart.chart, NULL,
                      std::move(deathMix)),
        new FadeOutPixelTransitionEffect());
  } else {
    auto evaluation = scores[localPlayerId]->evaluate();
    auto grade = evaluation->getGrade();
    u8 currentGrade =
        SAVEFILE_read8(SRAM->deathMixProgress.grades[chart->difficulty]);
    if (firstTime || grade < currentGrade)
      SAVEFILE_write8(SRAM->deathMixProgress.grades[chart->difficulty], grade);

    engine->transitionIntoScene(
        new DanceGradeScene(engine, fs, std::move(evaluation), NULL, false,
                            true),
        new PixelTransitionEffect());
  }
}

void SongScene::processModsLoad() {
  if ($isMultiplayer)
    return;

  if (GameState.mods.pixelate == PixelateOpts::pFIXED ||
      GameState.mods.pixelate == PixelateOpts::pBLINK_OUT)
    targetMosaic = 4;
  else if (GameState.mods.pixelate == PixelateOpts::pBLINK_IN)
    targetMosaic = 0;

  if (SAVEFILE_getGameMode() == GameMode::IMPOSSIBLE)
    setRate(1);
}

void SongScene::processModsBeat() {
  if ($isMultiplayer)
    return;

  if (GameState.mods.autoMod) {
    autoModCounter++;
    if (autoModCounter == autoModDuration) {
      autoModCounter = 0;
      autoModDuration = qran_range(1, 5);

      auto previousColorFilter = GameState.mods.colorFilter;
      GameState.mods.pixelate =
          qran_range(1, 101) > 75
              ? (qran_range(1, 101) > 50 ? PixelateOpts::pBLINK_IN
                                         : PixelateOpts::pFIXED)
              : PixelateOpts::pOFF;
      GameState.mods.jump = qran_range(1, 101) > 50 && !$isDouble
                                ? JumpOpts::jLINEAR
                                : JumpOpts::jOFF;
      GameState.mods.reduce =
          GameState.mods.autoMod == AutoModOpts::aINSANE
              ? (qran_range(1, 101) > 50
                     ? (qran_range(1, 101) > 50 ? ReduceOpts::rLINEAR
                                                : ReduceOpts::rMICRO)
                     : ReduceOpts::rOFF)
              : ReduceOpts::rMICRO;
      GameState.mods.bounce = BounceOpts::bALL;
      GameState.mods.colorFilter =
          qran_range(1, 101) > 50 ? static_cast<ColorFilter>(qran_range(1, 17))
                                  : ColorFilter::NO_FILTER;

      mosaic = targetMosaic = 0;
      processModsLoad();
      updateGameX();
      updateGameY();

      if (GameState.mods.colorFilter != previousColorFilter)
        reapplyFilter(GameState.mods.colorFilter);
    }
  }

  if (GameState.mods.pixelate == PixelateOpts::pBLINK_IN)
    mosaic = 6;
  else if (GameState.mods.pixelate == PixelateOpts::pBLINK_OUT)
    mosaic = 0;
  else if (GameState.mods.pixelate == PixelateOpts::pRANDOM) {
    auto previousTargetMosaic = targetMosaic;
    targetMosaic = qran_range(1, 5);
    if (previousTargetMosaic == targetMosaic)
      mosaic = 0;
  }

  if (GameState.mods.jump == JumpOpts::jRANDOM) {
    int random = qran_range(0, GAME_POSITION_X[GamePosition::RIGHT] + 1);
    GameState.positionX[0] = random;
    updateGameX();
    pixelBlink->blink();
  }

  if (GameState.mods.reduce == ReduceOpts::rRANDOM) {
    int random = qran_range(0, REDUCE_MOD_POSITION_Y);
    GameState.positionY = REDUCE_MOD_POSITION_Y - random;
    updateGameY();
    pixelBlink->blink();
  }

  if (GameState.mods.bounce != BounceOpts::bOFF)
    bounceDirection *= -1;

  if (GameState.mods.speedHack == SpeedHackOpts::hRANDOM)
    chartReaders[0]->setMultiplier(qran_range(3, 6));
}

void SongScene::processModsTick() {
  if ($isMultiplayer)
    return;

  if (GameState.mods.jump == JumpOpts::jLINEAR) {
    GameState.positionX[0] += jumpDirection * (1 + (blinkFrame / 2));

    int endPosition = GAME_POSITION_X[GamePosition::RIGHT];
    if (GameState.positionX[0] >= (int)endPosition ||
        GameState.positionX[0] <= 0) {
      GameState.positionX[0] = GameState.positionX[0] <= 0 ? 0 : endPosition;
      jumpDirection *= -1;
    }

    updateGameX();
  }

  if (GameState.mods.reduce == ReduceOpts::rLINEAR) {
    GameState.positionY += reduceDirection * (1 + (blinkFrame / 2));

    if (GameState.positionY >= REDUCE_MOD_POSITION_Y ||
        GameState.positionY <= 0) {
      GameState.positionY =
          GameState.positionY <= 0 ? 0 : REDUCE_MOD_POSITION_Y;
      reduceDirection *= -1;
    }
    updateGameY();
  } else if (GameState.mods.reduce == ReduceOpts::rMICRO) {
    GameState.positionY = BOUNCE_STEPS[blinkFrame];
    updateGameY();
  }
}

u8 SongScene::processPixelateMod() {
  u8 minMosaic = 0;

  switch (GameState.mods.pixelate) {
    case PixelateOpts::pOFF:
      break;
    case PixelateOpts::pLIFE:
      minMosaic = lifeBars[0]->getMosaicValue();
      break;
    case PixelateOpts::pFIXED:
    case PixelateOpts::pBLINK_IN:
    case PixelateOpts::pBLINK_OUT:
    case PixelateOpts::pRANDOM:
      waitMosaic = !waitMosaic;

      if (!waitMosaic) {
        if (targetMosaic > mosaic)
          mosaic++;
        else if (mosaic > targetMosaic)
          mosaic--;
      }

      minMosaic = mosaic;
      break;
  }

  EFFECT_setMosaic(minMosaic);
  return minMosaic;
}

void SongScene::processTrainingModeMod() {
  // Rate down
  if ((bInput->hasBeenPressedNow() && selectInput->getIsPressed()) ||
      (bInput->getIsPressed() && selectInput->hasBeenPressedNow())) {
    selectInput->setHandledFlag(true);

    if (setRate(rate - 1))
      pixelBlink->blink();
  }

  // Rate up
  if ((bInput->hasBeenPressedNow() && startInput->getIsPressed()) ||
      (bInput->getIsPressed() && startInput->hasBeenPressedNow())) {
    startInput->setHandledFlag(true);

    if (setRate(rate + 1))
      pixelBlink->blink();
  }

  // Fast forward
  if (aInput->getIsPressed() && startInput->getIsPressed()) {
    startInput->setHandledFlag(true);

    judge->disable();
    player_seek(PlaybackState.msecs + 100);
    RUMBLE_stop();
  } else
    judge->enable();

  // Multiplier down
  if (selectInput->hasBeenReleasedNow()) {
    if (!selectInput->getHandledFlag()) {
      if (chartReaders[0]->setMultiplier(chartReaders[0]->getMultiplier() - 1))
        pixelBlink->blink();
    }
    selectInput->setHandledFlag(false);
  }

  // Multiplier up
  if (startInput->hasBeenReleasedNow()) {
    if (!startInput->getHandledFlag()) {
      if (chartReaders[0]->setMultiplier(chartReaders[0]->getMultiplier() + 1))
        pixelBlink->blink();
    }
    startInput->setHandledFlag(false);
  }

  // Reset handled flag
  if (startInput->hasBeenPressedNow() && !aInput->getIsPressed() &&
      !bInput->getIsPressed())
    startInput->setHandledFlag(false);
}

void SongScene::processMultiplayerUpdates() {
  u32 keys =
      SYNC_MSG_KEYS_BUILD(arrowHolders[localBaseIndex + 0]->getIsPressed(),
                          arrowHolders[localBaseIndex + 1]->getIsPressed(),
                          arrowHolders[localBaseIndex + 2]->getIsPressed(),
                          arrowHolders[localBaseIndex + 3]->getIsPressed(),
                          arrowHolders[localBaseIndex + 4]->getIsPressed());

  if (syncer->isPlaying() &&
      linkUniversal->getMode() == LinkUniversal::Mode::LINK_WIRELESS)
    linkUniversal->linkWireless->SEND_ARROWS = keys;
  else
    syncer->send(SYNC_EVENT_KEYS, keys);

  auto remoteId = syncer->getRemotePlayerId();
  bool remoteArrows[ARROWS_TOTAL];
  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    remoteArrows[i] = arrowHolders[remoteBaseIndex + i]->getIsPressed();

  linkUniversal->sync();

  if (syncer->isPlaying() &&
      linkUniversal->getMode() == LinkUniversal::Mode::LINK_WIRELESS) {
    u16 keys = linkUniversal->linkWireless->RECEIVE_ARROWS;
    for (u32 i = 0; i < ARROWS_TOTAL; i++)
      remoteArrows[i] = SYNC_MSG_KEYS_DIRECTION(keys, i);
  }

  while (syncer->isPlaying() && linkUniversal->canRead(remoteId)) {
    u16 message = linkUniversal->read(remoteId);
    u8 event = SYNC_MSG_EVENT(message);
    u16 payload = SYNC_MSG_PAYLOAD(message);

    if (!syncer->isMaster() && (message & SYNC_AUDIO_CHUNK_HEADER) != 0) {
      syncer->$currentAudioChunk = message & ~SYNC_AUDIO_CHUNK_HEADER;
      syncer->clearTimeout();
      continue;
    }

    switch (event) {
      case SYNC_EVENT_KEYS: {
        for (u32 i = 0; i < ARROWS_TOTAL; i++)
          remoteArrows[i] = SYNC_MSG_KEYS_DIRECTION(payload, i);

        syncer->clearTimeout();
        break;
      }
      case SYNC_EVENT_FEEDBACK: {
        if (isCoop() && syncer->isMaster()) {
          syncer->registerTimeout();
          break;
        }

        auto feedbackType =
            static_cast<FeedbackType>(SYNC_MSG_FEEDBACK_TYPE(payload));
        bool isLong = SYNC_MSG_FEEDBACK_IS_LONG(payload);

        scores[remoteId]->update(feedbackType, isLong);

        syncer->clearTimeout();
        break;
      }
      case SYNC_EVENT_MULTIPLIER_CHANGE: {
        if ($isVs)
          chartReaders[remoteId]->setMultiplier(payload);
        else {
          if (chartReaders[0]->setMultiplier(payload))
            pixelBlink->blink();
        }

        syncer->clearTimeout();
        break;
      }
      case SYNC_EVENT_STAGE_END: {
        if (payload)
          finishAndGoToEvaluation();
        else
          onStageBreak(remoteId);

        syncer->clearTimeout();
        break;
      }
      case SYNC_EVENT_ABORT: {
        onAbort();

        syncer->clearTimeout();
        break;
      }
      default: {
        syncer->registerTimeout();
      }
    }
  }

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders[remoteBaseIndex + i]->setIsPressed(remoteArrows[i]);

  if (syncer->$resetFlag && !engine->isTransitioning()) {
    syncer->send(SYNC_EVENT_ABORT, 0);
    syncer->clearTimeout();
    onAbort();
    return;
  }
}

bool SongScene::setRate(int rate) {
  int oldRate = this->rate;
  this->rate = max(min(rate, RATE_LEVELS), -RATE_LEVELS);
  if (this->rate == oldRate)
    return false;

  player_setRate(rate);
  chartReaders[0]->syncRate(RATE_LEVELS, rate);
  return true;
}

void SongScene::unload() {
  player_stop();
  RUMBLE_stop();

  if ($isMultiplayer)
    syncer->resetSongState();
  else
    IOPORT_sdLow();
}

SongScene::~SongScene() {
  arrowHolders.clear();
  fakeHeads.clear();
  SONG_free(song);
}

inline void backupPalettes(void (*onProgress)(u32 progress)) {
  for (u32 filter = 0; filter < TOTAL_COLOR_FILTERS; filter++) {
    auto colorFilter = static_cast<ColorFilter>(filter);

    COLOR* src = (COLOR*)MEM_PAL;
    for (u32 i = 0; i < PALETTE_MAX_SIZE * 2; i++)
      paletteBackups[filter][i] = SCENE_transformColor(src[i], colorFilter);
    onProgress(filter);
  }
}

inline void reapplyFilter(ColorFilter colorFilter) {
  COLOR* dest = (COLOR*)MEM_PAL;
  for (u32 i = 0; i < PALETTE_MAX_SIZE * 2; i++)
    dest[i] = paletteBackups[colorFilter][i];
}
