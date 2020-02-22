#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
  #include "player/gsm_main.h"
}

std::shared_ptr<GBAEngine> engine { new GBAEngine() };

int main() {
  SongScene* songScene = new SongScene(engine);
  engine->setScene(songScene);

  maino([]() {
    engine->update();
  });

  return 0;
}
