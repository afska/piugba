#ifndef SETTINGS_SCENE_H
#define SETTINGS_SCENE_H

#include "base/MenuScene.h"

class SettingsScene : public MenuScene {
 public:
  SettingsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

 protected:
  u16 getCloseKey() override;
  u32 getOptionsCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected, int direction) override;
};

#endif  // SETTINGS_SCENE_H
