#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
#include "player/player.h"
#include "utils/gbfs.h"
}

static std::shared_ptr<GBAEngine> engine{new GBAEngine()};

int main() {
  SongScene* songScene = new SongScene(engine);
  engine->setScene(songScene);

  player_init();

  player_play("beethoven-virus-full.gsm");
  engine->getTimer()->start();

  player_forever([]() {
    engine->getTimer()->onvblank();
    engine->update();
  });

  return 0;
}
