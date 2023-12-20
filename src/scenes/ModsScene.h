#ifndef MODS_SCENE_H
#define MODS_SCENE_H

#include "base/MenuScene.h"

const u32 TOTAL_COLOR_FILTERS = 17;

class ModsScene : public MenuScene {
 public:
  ModsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

 protected:
  u16 getCloseKey() override;
  u32 getOptionsCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected, int direction) override;
};

#endif  // MODS_SCENE_H
