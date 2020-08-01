#ifndef TALK_SCENE_H
#define TALK_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <functional>
#include <string>

#include "objects/ArrowSelector.h"
#include "objects/Instructor.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class TalkScene : public Scene {
 public:
  TalkScene(std::shared_ptr<GBAEngine> engine,
            const GBFS_FILE* fs,
            std::string message,
            std::function<void()> onComplete);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;

  std::unique_ptr<Instructor> instructor;
  std::unique_ptr<ArrowSelector> nextButton;
  std::string message;
  std::function<void()> onComplete;

  void setUpSpritesPalette();
  void setUpBackground();
};

#endif  // TALK_SCENE_H
