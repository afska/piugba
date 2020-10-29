#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "gameplay/ChartReader.h"
#include "gameplay/Judge.h"
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
  SongScene(std::shared_ptr<GBAEngine> engine,
            const GBFS_FILE* fs,
            Song* song,
            Chart* chart);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

  ~SongScene();

 private:
  u32 init = 0;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  Song* song;
  Chart* chart;
  std::unique_ptr<ChartReader> chartReader;
  std::unique_ptr<Judge> judge;
  std::unique_ptr<LifeBar> lifeBar;
  std::unique_ptr<Score> score;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::vector<std::unique_ptr<Arrow>> fakeHeads;
  std::unique_ptr<ObjectPool<Arrow>> arrowPool;
  std::unique_ptr<InputHandler> speedUpInput;
  std::unique_ptr<InputHandler> speedDownInput;
  std::unique_ptr<InputHandler> keyA;
  std::unique_ptr<InputHandler> keyB;
  int rate = 0;
  u32 blinkFrame = 0;
  u8 targetMosaic = 0;
  u8 mosaic = 0;
  bool waitMosaic = true;
  int jumpDirection = 1;
  int reduceDirection = -1;

  void setUpPalettes();
  void setUpBackground();
  void setUpArrows();

  void updateArrowHolders();
  void updateArrows();
  void updateFakeHeads();
  void updateGameX();
  void updateGameY();
  void processKeys(u16 keys);
  void finishAndGoToEvaluation();

  void processModsLoad();
  void processModsTick();
  void processModsBeat();
  u8 processPixelateMod();
  void processTrainingModeMod();
  bool setRate(int rate);

  void unload();
};

#endif  // SONG_SCENE_H
