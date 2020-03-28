#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>
#include "gameplay/ChartReader.h"
#include "gameplay/models/Song.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/score/Feedback.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectQueue.h"

extern "C" {
#include "utils/gbfs.h"
}

class SongScene : public Scene {
 public:
  u32 msecs = 0;

  SongScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
      : Scene(engine) {
    this->fs = fs;
  }

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

  ~SongScene();

 private:
  const GBFS_FILE* fs;
  std::unique_ptr<ChartReader> chartReader;
  std::unique_ptr<Background> bg;
  std::unique_ptr<Score> score;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::unique_ptr<ObjectQueue<Arrow>> arrowQueue;
  int lastBeat = 0;

  void setUpBackground();
  void setUpArrows();
  void updateArrowHolders();
  void updateArrows(u32 millis);
  void processKeys(u16 keys);
};

#endif  // SONG_SCENE_H
