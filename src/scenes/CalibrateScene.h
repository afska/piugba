#ifndef CALIBRATE_SCENE_H
#define CALIBRATE_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <functional>

#include "objects/ui/ArrowSelector.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class CalibrateScene : public Scene {
 public:
  explicit CalibrateScene(std::shared_ptr<GBAEngine> engine,
                          const GBFS_FILE* fs,
                          std::function<void()> onFinish = NULL);

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

  std::unique_ptr<ArrowSelector> calibrateButton;
  std::unique_ptr<ArrowSelector> resetButton;
  std::unique_ptr<ArrowSelector> saveButton;
  bool isMeasuring = false;
  bool hasDoneChanges = false;
  int measuredLag = 0;

  void setUpSpritesPalette();
  void setUpBackground();

  void processKeys(u16 keys);
  void printTitle();
  void calibrate();
  void start();
  void finish();
  void save();
  void goBack();
};

#endif  // CALIBRATE_SCENE_H
