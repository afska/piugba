#ifndef MULTIPLAYER_LOBBY_SCENE_H
#define MULTIPLAYER_LOBBY_SCENE_H

#include "base/TextScene.h"

class MultiplayerLobbyScene : public TextScene {
 public:
  MultiplayerLobbyScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
      : TextScene(engine, fs) {}

 protected:
  void load() override;
  void tick(u16 keys) override;

 private:
  u32 messageId = 0;
};

#endif  // MULTIPLAYER_LOBBY_SCENE_H
