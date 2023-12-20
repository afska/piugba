#ifndef MENU_SCENE_H
#define MENU_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <string>

#include "objects/base/InputHandler.h"
#include "objects/ui/ArrowSelector.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class MenuScene : public Scene {
 public:
  MenuScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 protected:
  std::unique_ptr<Background> bg;
  const GBFS_FILE* fs;
  u32 selected = 0;

  virtual u16 getCloseKey() = 0;
  virtual u32 getOptionsCount() = 0;
  virtual void loadBackground(u32 id) = 0;
  virtual void printOptions() = 0;
  virtual bool selectOption(u32 selected, int direction) = 0;
  virtual void close();

  void printOption(u32 id,
                   std::string name,
                   std::string value,
                   u32 row,
                   bool highlightChange = false,
                   std::string defaultValue = "");
  u8 change(u8 value, u8 optionsCount, int direction);

 private:
  bool hasStarted = false;
  std::unique_ptr<PixelBlink> pixelBlink;

  std::unique_ptr<ArrowSelector> selectButton;
  std::unique_ptr<ArrowSelector> backButton;
  std::unique_ptr<ArrowSelector> nextButton;
  std::unique_ptr<InputHandler> closeInput;
  std::unique_ptr<InputHandler> incrementInput;
  std::unique_ptr<InputHandler> decrementInput;
  bool isUsingGBAStyle = false;
  bool blockButtons = false;

  void setUpSpritesPalette();
  void setUpBackground();

  void processKeys(u16 keys);
  void processSelection();
  void printMenu();
  void move(int direction);
  void select(int direction);
  u8 increment(u8 value, u8 optionsCount);
  u8 decrement(u8 value, u8 optionsCount);
};

#endif  // MENU_SCENE_H
