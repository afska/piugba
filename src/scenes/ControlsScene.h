#ifndef CONTROLS_SCENE_H
#define CONTROLS_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/ArrowSelector.h"
#include "objects/ArrowTutorial.h"
#include "objects/Instructor.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class ControlsScene : public Scene {
 public:
  ControlsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  std::unique_ptr<Instructor> instructor;
  std::vector<std::unique_ptr<ArrowSelector>> buttons;
  std::vector<std::unique_ptr<ArrowTutorial>> comboArrows;
  u32 comboStep = 0;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();

  void processKeys(u16 keys);
  void processCombo();
  void advanceCombo();
  void resetCombo();
};

#endif  // CONTROLS_SCENE_H