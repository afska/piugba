#ifndef ADMIN_SCENE_H
#define ADMIN_SCENE_H

#include "base/MenuScene.h"

class AdminScene : public MenuScene {
 public:
  AdminScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

 protected:
  u16 getCloseKey() override;
  u32 getOptionsCount() override;
  void loadBackground(u32 id) override;
  void printOptions() override;
  bool selectOption(u32 selected) override;
  void close() override;
};

#endif  // ADMIN_SCENE_H
