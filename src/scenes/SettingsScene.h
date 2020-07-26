#ifndef SETTINGS_SCENE_H
#define SETTINGS_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/ArrowSelector.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class SettingsScene : public Scene {
 public:
  SettingsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  // TODO: ARROWS
  // std::unique_ptr<ArrowSelector> calibrateButton;
  // std::unique_ptr<ArrowSelector> resetButton;
  // std::unique_ptr<ArrowSelector> saveButton;

  void setUpSpritesPalette();
  void setUpBackground();

  void processKeys(u16 keys);
  void printMenu();
};

#endif  // SETTINGS_SCENE_H
