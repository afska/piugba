#ifndef START_SCENE_H
#define START_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/base/InputHandler.h"
#include "objects/ui/Button.h"
#include "ui/Darkener.h"
#include "utils/PixelBlink.h"
#include "utils/pool/ObjectPool.h"

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
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::unique_ptr<ObjectPool<Arrow>> arrowPool;
  bool isExpanded = false;
  int lastBeat = 0;
  u8 selectedMode = 0;
  u8 darkenerOpacity;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpButtons();
  void setUpGameAnimation();

  void animateBpm();
  void animateArrows();

  void printTitle();
  void processKeys(u16 keys);
  void processSelectionChange();
  void goToGame();

  ~StartScene();
};

#endif  // START_SCENE_H
