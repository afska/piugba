#ifndef STAGE_BREAK_SCENE_H
#define STAGE_BREAK_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

class StageBreakScene : public Scene {
 public:
  StageBreakScene(std::shared_ptr<GBAEngine> engine);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

  ~StageBreakScene();

 private:
};

#endif  // STAGE_BREAK_SCENE_H
