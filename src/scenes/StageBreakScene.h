#ifndef STAGE_BREAK_SCENE_H
#define STAGE_BREAK_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/Instructor.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class StageBreakScene : public Scene {
 public:
  StageBreakScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Instructor> instructor;
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;

  void setUpSpritesPalette();
  void setUpBackground();
};

#endif  // STAGE_BREAK_SCENE_H
