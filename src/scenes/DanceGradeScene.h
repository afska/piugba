#ifndef DANCE_GRADE_SCENE_H
#define DANCE_GRADE_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/score/Grade.h"

class DanceGradeScene : public Scene {
 public:
  DanceGradeScene(std::shared_ptr<GBAEngine> engine);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  std::unique_ptr<Grade> grade;

  void setUpSpritesPalette();
};

#endif  // DANCE_GRADE_SCENE_H
