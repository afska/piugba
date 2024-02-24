#ifndef SETTINGS_SCENE_H
#define SETTINGS_SCENE_H

#include "base/MenuScene.h"

class SettingsScene : public MenuScene {
 public:
  SettingsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

 protected:
  u16 getCloseKey() override;
  u32 getOptionCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected, int direction) override;
  void close() override;

 private:
  bool quitToAdminMenu = false;
};

#endif  // SETTINGS_SCENE_H
