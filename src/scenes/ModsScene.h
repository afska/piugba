#ifndef MODS_SCENE_H
#define MODS_SCENE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "base/MenuScene.h"

class ModsScene : public MenuScene {
 public:
  ModsScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

 protected:
  u16 getCloseKey() override;
  u32 getOptionsCount() override;
  void printOptions() override;
  bool selectOption(u32 selected) override;
};

#endif  // MODS_SCENE_H
