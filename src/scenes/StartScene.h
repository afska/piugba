#ifndef START_SCENE_H
#define START_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/InputHandler.h"
#include "objects/ui/Button.h"
#include "ui/Darkener.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class StartScene : public Scene {
 public:
  StartScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  std::unique_ptr<Darkener> darkener;
  std::vector<std::unique_ptr<Button>> buttons;
  std::vector<std::unique_ptr<InputHandler>> inputHandlers;
  u32 lastBeat = 0;
  u32 selectedMode = 0;
  u32 darkenerOpacity;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpButtons();
  void setUpGameAnimation();

  void animateBpm();

  void processKeys(u16 keys);
  void processSelectionChange();
};

#endif  // START_SCENE_H
