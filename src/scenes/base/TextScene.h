#ifndef TEXT_SCENE_H
#define TEXT_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <string>

#include "objects/ui/Instructor.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class TextScene : public Scene {
 public:
  TextScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  virtual std::vector<Sprite*> sprites() override;

  virtual void load() override;
  virtual void tick(u16 keys) override;

 protected:
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;

  void write(std::string text);
  inline bool hasFinished() { return row == lines.size(); }

  ~TextScene();

 private:
  bool hasStarted = false;

  std::unique_ptr<Instructor> instructor;
  std::vector<std::string> lines;
  u32 col = 0;
  u32 row = 0;
  bool wait = true;

  void setUpSpritesPalette();
  void setUpBackground();

  void alignText();
  void autoWrite();
};

#endif  // TEXT_SCENE_H
