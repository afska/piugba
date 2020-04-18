#ifndef START_SCENE_H
#define START_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

class StartScene : public Scene {
 public:
  StartScene(std::shared_ptr<GBAEngine> engine);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

  ~StartScene();

 private:
  std::unique_ptr<Background> bg;
  int i = 0;  // TODO: REMOVE
};

#endif  // START_SCENE_H
