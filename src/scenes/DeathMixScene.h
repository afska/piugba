#ifndef DEATH_MIX_SCENE_H
#define DEATH_MIX_SCENE_H

#include "TalkScene.h"
#include "gameplay/SequenceMessages.h"
#include "gameplay/save/SaveFile.h"
#include "objects/base/InputHandler.h"
#include "objects/ui/Difficulty.h"
#include "objects/ui/Multiplier.h"
#include "objects/ui/NumericProgress.h"

class DeathMixScene : public TalkScene {
 public:
  DeathMixScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
      : TalkScene(
            engine,
            fs,
            MODE_DEATH_MIX,
            [this](u16 keys) { confirm(keys); },
            true) {}

  virtual std::vector<Sprite*> sprites() override;

 protected:
  void load() override;
  void tick(u16 keys) override;

 private:
  std::unique_ptr<Multiplier> multiplier;
  std::unique_ptr<Difficulty> difficulty;
  std::unique_ptr<NumericProgress> progress;
  std::unique_ptr<ArrowSelector> backButton;
  std::unique_ptr<ArrowSelector> nextButton;
  std::unique_ptr<InputHandler> settingsMenuInput;

  void setUpSpritesPalette();

  void processKeys(u16 keys);
  void processDifficultyChangeEvents();
  void processMenuEvents();
  void confirm(u16 keys);
};

#endif  // DEATH_MIX_SCENE_H
