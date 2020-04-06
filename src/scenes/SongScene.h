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
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

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
  const GBFS_FILE* fs;
  Song* song;
  Chart* chart;
  std::unique_ptr<ChartReader> chartReader;
  std::unique_ptr<Judge> judge;
  std::unique_ptr<LifeBar> lifeBar;
  std::unique_ptr<Background> bg;
  std::unique_ptr<Score> score;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::unique_ptr<ObjectPool<Arrow>> arrowPool;
  u32 msecs = 0;

  void setUpPalettes();
  void setUpBackground();
  void setUpArrows();
  void updateArrowHolders();
  void updateArrows();
  void processKeys(u16 keys);
};

#endif  // SONG_SCENE_H
