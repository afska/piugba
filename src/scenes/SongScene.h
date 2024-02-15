#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "gameplay/ChartReader.h"
#include "gameplay/DeathMix.h"
#include "gameplay/Judge.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/LifeBar.h"
#include "objects/base/InputHandler.h"
#include "objects/score/Score.h"
#include "utils/PixelBlink.h"
#include "utils/pool/ObjectPool.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class SongScene : public Scene {
 public:
  explicit SongScene(std::shared_ptr<GBAEngine> engine,
                     const GBFS_FILE* fs,
                     Song* song,
                     Chart* chart,
                     Chart* remoteChart = NULL,
                     std::unique_ptr<DeathMix> deathMix = NULL);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;
  void render() override;

  ~SongScene();

 private:
  u32 init = 0;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  Song* song;
  Chart* chart;
  Chart* remoteChart;
  std::unique_ptr<ChartReader> chartReaders[GAME_MAX_PLAYERS];
  std::unique_ptr<LifeBar> lifeBars[GAME_MAX_PLAYERS];
  std::array<std::unique_ptr<Score>, GAME_MAX_PLAYERS> scores;
  std::unique_ptr<Judge> judge;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::vector<std::unique_ptr<Arrow>> fakeHeads;
  std::unique_ptr<ObjectPool<Arrow>> arrowPool;
  std::unique_ptr<InputHandler> startInput;
  std::unique_ptr<InputHandler> selectInput;
  std::unique_ptr<InputHandler> aInput;
  std::unique_ptr<InputHandler> bInput;
  std::unique_ptr<DeathMix> deathMix;
  bool $isMultiplayer, $isDouble, $isVs, $isSinglePlayerDouble,
      $isVsDifferentLevels;
  u32 platformCount, playerCount, localBaseIndex, remoteBaseIndex,
      localPlayerId;
  int rate = 0;
  u32 blinkFrame = 0;
  u8 targetMosaic = 0;
  u8 mosaic = 0;
  bool waitMosaic = true;
  int jumpDirection = 1;
  int reduceDirection = 1;
  int bounceDirection = -1;
  int rumbleBeatFrame = -1;
  int rumbleIdleFrame = 0;
  u32 autoModDuration = 1;
  u32 autoModCounter = 0;
  u32 lastDownLeftKeys = 0;
  u32 lastUpLeftKeys = 0;
  u32 lastCenterKeys = 0;

  inline void setUpGameConfig() {
    $isMultiplayer = isMultiplayer();
    $isDouble = isDouble();
    $isVs = isVs();
    $isSinglePlayerDouble = isSinglePlayerDouble();
    $isVsDifferentLevels = remoteChart->level != chart->level;
    platformCount = isMultiplayer() || isSinglePlayerDouble() ? 2 : 1;
    playerCount = 1 + isVs();
    localBaseIndex = isMultiplayer()
                         ? getBaseIndexFromPlayerId(syncer->getLocalPlayerId())
                         : 0;
    remoteBaseIndex = getBaseIndexFromPlayerId(syncer->getRemotePlayerId());
    localPlayerId = isVs() ? syncer->getLocalPlayerId() : 0;
  }

  inline ArrowDirection getDirectionFromIndex(u32 index) {
    return static_cast<ArrowDirection>($isDouble ? index
                                                 : DivMod(index, ARROWS_TOTAL));
  }

  inline u8 getPlayerIdFromIndex(u32 index) {
    return $isDouble ? 0 : (index >= ARROWS_TOTAL ? 1 : 0);
  }

  inline u8 getBaseIndexFromPlayerId(u8 playerId) {
    return playerId * ARROWS_TOTAL;
  }

  void setUpPalettes();
  void setUpBackground();
  void setUpArrows();
  void initializeBackground();
  bool initializeGame(u16 keys);

  void updateArrowHolders();
  void updateArrows();
  void updateBlink();
  void updateFakeHeads();
  void updateScoresAndLifebars();
  void updateGameX();
  void updateGameY();
  void updateRumble();
  void animateWinnerLifeBar();
  void drawVideo();
  void processKeys(u16 keys);

  void onNewBeat(bool isAnyKeyPressed);
  void onStageBreak(u8 playerId);
  void onStagePass();
  void onAbort();
  void breakStage();
  void finishAndGoToEvaluation();
  void continueDeathMix();

  void processModsLoad();
  void processModsTick();
  void processModsBeat();
  u8 processPixelateMod();
  void processTrainingModeMod();
  void processMultiplayerUpdates();
  bool setRate(int rate);

  void unload();
};

#endif  // SONG_SCENE_H
