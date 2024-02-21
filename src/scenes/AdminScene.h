#ifndef ADMIN_SCENE_H
#define ADMIN_SCENE_H

#include "base/MenuScene.h"

class AdminScene : public MenuScene {
 public:
  AdminScene(std::shared_ptr<GBAEngine> engine,
             const GBFS_FILE* fs,
             bool withSound = false);

 protected:
  u16 getCloseKey() override;
  u32 getOptionCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected, int direction) override;
  void close() override;

 private:
  int submenu = -1;
  u32 totalOffsets = 0;
  bool withSound;
};

#endif  // ADMIN_SCENE_H
