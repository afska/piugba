#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
#include "player/player.h"
#include "utils/gbfs.h"
}

std::shared_ptr<GBAEngine> engine{new GBAEngine()};
SongScene* songScene;

int main() {
  songScene = new SongScene(engine);
  engine->setScene(songScene);

  player_init();
  player_play("beethoven-virus-full.gsm");
  player_forever([](u32 msecs) {
    songScene->setMsecs(msecs);

    engine->update();
  });

  return 0;
}
