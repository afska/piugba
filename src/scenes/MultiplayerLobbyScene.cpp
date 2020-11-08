#include "MultiplayerLobbyScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "SelectionScene.h"
#include "StartScene.h"
#include "gameplay/Key.h"
#include "utils/SceneUtils.h"

std::string messages[] = {"Connecting...\r\n\r\n(Press SELECT to cancel)",
                          "ERROR:\r\nToo many players!",
                          "ERROR:\r\nROM IDs don't match!",
                          "ERROR:\r\nMixed game modes!"};

void MultiplayerLobbyScene::load() {
  TextScene::load();

  syncer->initialize(mode);
  refresh(0);
}

void MultiplayerLobbyScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (syncer->isReady()) {
    start();
    return;
  }

  if (keys & KEY_SELECT) {
    goBack();
    return;
  }

  refresh(syncer->getLastError());
  TextScene::tick(keys);
  TextStream::instance().clear();  // TODO: REMOVE
  SCENE_write(std::to_string(syncer->getState()) + "-" +
                  std::to_string(syncer->getLastError()) + "->" +
                  std::to_string(syncer->a),
              0);
  SCENE_write(std::to_string(_isBitHigh(REG_SIOCNT, LINK_BIT_READY)) + "-" +
                  std::to_string(_isBitHigh(REG_SIOCNT, LINK_BIT_ERROR)) + "-" +
                  std::to_string(linkConnection->_linkState._isOutOfSync()) +
                  "|||" +
                  std::to_string(linkConnection->_linkState.playerCount),
              0 + 1);
}

void MultiplayerLobbyScene::refresh(int newMessageId) {
  if (newMessageId != messageId) {
    messageId = newMessageId;
    write(messages[messageId]);
  }
}

void MultiplayerLobbyScene::start() {
  engine->transitionIntoScene(new SelectionScene(engine, fs),
                              new FadeOutScene(2));
}

void MultiplayerLobbyScene::goBack() {
  engine->transitionIntoScene(new StartScene(engine, fs), new FadeOutScene(2));
  syncer->initialize(SyncMode::SYNC_MODE_OFFLINE);
}
