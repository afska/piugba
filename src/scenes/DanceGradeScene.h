#ifndef DANCE_GRADE_SCENE_H
#define DANCE_GRADE_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/score/Grade.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class DanceGradeScene : public Scene {
 public:
  DanceGradeScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Grade> grade;
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;

  void setUpSpritesPalette();
  void setUpBackground();
};

#endif  // DANCE_GRADE_SCENE_H
