#ifndef CONTROLS_SCENE_H
#define CONTROLS_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <functional>

#include "objects/ui/ArrowSelector.h"
#include "objects/ui/ArrowTutorial.h"
#include "objects/ui/Instructor.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class ControlsScene : public Scene {
 public:
  ControlsScene(std::shared_ptr<GBAEngine> engine,
                const GBFS_FILE* fs,
                std::function<void()> onFinish);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;
  void render() override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;
  std::function<void()> onFinish;

  std::unique_ptr<Instructor> instructor;
  std::vector<std::unique_ptr<ArrowSelector>> buttons;
  std::vector<std::unique_ptr<ArrowTutorial>> comboArrows;
  u32 comboStep = 0;
  int bounceDirectionX = 0;
  int bounceDirectionY = 0;
  u32 bounceStep = 0;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();

  void processKeys(u16 keys);
  void processCombo();
  void advanceCombo();
  void resetCombo();

  ~ControlsScene();
};

#endif  // CONTROLS_SCENE_H
