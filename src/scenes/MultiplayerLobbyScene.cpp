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

#ifdef SENV_DEBUG
  if (DEBUTRACE_LINE == -1) {
    TextScene::tick(keys);
    DEBUTRACE_LINE++;
  }
#endif
#ifndef SENV_DEBUG
  TextScene::tick(keys);
#endif
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
