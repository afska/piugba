#ifndef SETTINGS_SCENE_H
#define SETTINGS_SCENE_H

#include "base/MenuScene.h"

class SettingsScene : public MenuScene {
 public:
  SettingsScene(std::shared_ptr<GBAEngine> engine,
                const GBFS_FILE* fs,
                u32 initialOption = 0);

 protected:
  u32 getOptionCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected, int direction) override;
  void close() override;

 private:
  bool quitToAdminMenu = false;
  bool initialOption;
};

#endif  // SETTINGS_SCENE_H
