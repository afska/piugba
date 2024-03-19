#ifndef MULTIPLAYER_LOBBY_SCENE_H
#define MULTIPLAYER_LOBBY_SCENE_H

#include "base/TextScene.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/ui/Explosion.h"

class MultiplayerLobbyScene : public TextScene {
 public:
  MultiplayerLobbyScene(std::shared_ptr<GBAEngine> engine,
                        const GBFS_FILE* fs,
                        SyncMode mode)
      : TextScene(engine, fs) {
    this->mode = mode;
  }

  virtual std::vector<Sprite*> sprites() override;

 protected:
  void load() override;
  void tick(u16 keys) override;

 private:
  SyncMode mode;
  int messageId = -1;
  bool canExit = false;

  std::unique_ptr<Explosion> loadingIndicator;

  void refresh(int newMessageId);
  void start();
  void goBack();
};

#endif  // MULTIPLAYER_LOBBY_SCENE_H
