#ifndef TALK_SCENE_H
#define TALK_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <functional>
#include <string>

#include "objects/ui/ArrowSelector.h"
#include "objects/ui/Instructor.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class TalkScene : public Scene {
 public:
  bool withButton = true;

  TalkScene(std::shared_ptr<GBAEngine> engine,
            const GBFS_FILE* fs,
            std::string message,
            std::function<void(u16 keys)> onKeyPress);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  bool hasStarted = false;
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;
  std::function<void(u16 keys)> onKeyPress;

  std::unique_ptr<Instructor> instructor;
  std::unique_ptr<ArrowSelector> nextButton;
  std::vector<std::string> lines;
  u32 col = 0;
  u32 row = 0;
  bool wait = true;

  inline bool hasFinished() { return row == lines.size(); }

  void setUpSpritesPalette();
  void setUpBackground();

  void alignText();
  void autoWrite();
};

#endif  // TALK_SCENE_H
