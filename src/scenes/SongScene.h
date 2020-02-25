#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>
#include "objects/DanceAnimation.h"

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
  std::unique_ptr<Sprite> a1;
  std::unique_ptr<Sprite> a2;
  std::unique_ptr<Sprite> a3;
  std::unique_ptr<Sprite> a4;
  std::unique_ptr<Sprite> a5;
  std::unique_ptr<Sprite> aa1;
  std::unique_ptr<Sprite> aa2;
  std::unique_ptr<Sprite> aa3;
  std::unique_ptr<Sprite> aa4;
  std::unique_ptr<Sprite> aa5;
  u32 msecs = 0;
  bool started = false;
  u32 lastBeat = 0;
};

#endif  // SONG_SCENE_H
