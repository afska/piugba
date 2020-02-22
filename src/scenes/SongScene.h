#ifndef SONG_SCENE_H
#define SONG_SCENE_H

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>
#include <libgba-sprite-engine/background/text_stream.h>

class SongScene : public Scene {
 public:
  SongScene(std::shared_ptr<GBAEngine> engine) : Scene(engine) {}

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  std::unique_ptr<Sprite> animation;
};

#endif  // SONG_SCENE_H
