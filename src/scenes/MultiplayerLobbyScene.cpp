#include "MultiplayerLobbyScene.h"

#include "gameplay/Key.h"
#include "gameplay/multiplayer/Syncer.h"
#include "utils/SceneUtils.h"

std::string messages[] = {"Connecting...\r\n\r\n(Press SELECT to cancel)"};

void MultiplayerLobbyScene::load() {
  TextScene::load();

  syncer->setMode(SyncMode::SYNC_MODE_VS);  // TODO: Parameterize
  write(messages[messageId]);               // TODO: MOVE
}

void MultiplayerLobbyScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TextScene::tick(keys);

  if (keys & KEY_SELECT) {
    RegisterRamReset(RESET_REG | RESET_VRAM);
    SoftReset();
  }
}
