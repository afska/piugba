#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>
#include "objects/DanceAnimation.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"

class SongScene : public Scene {
 public:
  SongScene(std::shared_ptr<GBAEngine> engine) : Scene(engine) {}

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;
  void setMsecs(u32 msecs);

  void load() override;
  void tick(u16 keys) override;

 private:
  std::unique_ptr<Background> bg;
  std::unique_ptr<DanceAnimation> animation;
  std::vector<std::unique_ptr<Arrow>> arrows;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  u32 msecs = 0;
  bool started = false;
  u32 lastBeat = 0;

  void setUpBackground();
  void setUpArrowHolders();
  void updateArrowHolders();
  void updateArrows();
};

#endif  // SONG_SCENE_H
