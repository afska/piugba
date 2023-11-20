#ifndef START_SCENE_H
#define START_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/base/InputHandler.h"
#include "objects/ui/ArrowSelector.h"
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
  std::vector<std::unique_ptr<ArrowSelector>> inputs;
  std::vector<std::unique_ptr<ArrowHolder>> arrowHolders;
  std::unique_ptr<ObjectPool<Arrow>> arrowPool;
  bool isExpanded = false;
  bool wasNotPressingAdminCombo = false;
  int lastBeat = 0;
  int lastTick = 0;
  u8 selectedMode = 0;
  u8 darkenerOpacity;

  inline u32 getTickCount() {
    return SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::CRAZY)  ? 4
           : SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::HARD) ? 2
           : SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::NORMAL)
               ? 1
               : 1;
  }

  inline u32 getArrowSpeed() {
    return SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::CRAZY)  ? 6
           : SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::HARD) ? 5
           : SAVEFILE_didComplete(GameMode::CAMPAIGN, DifficultyLevel::NORMAL)
               ? 3
               : 3;
  }

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpInputs();
  void setUpButtons();
  void setUpGameAnimation();

  void animateBpm();
  void animateArrows();

  void printTitle();
  void processKeys(u16 keys);
  void processSelectionChange();
  void navigateToAdminMenuIfNeeded(u16 keys);
  bool isPressingAdminCombo(u16 keys);
  void goToGame();

  ~StartScene();
};

#endif  // START_SCENE_H
