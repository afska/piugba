#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

int main() {
  std::shared_ptr<GBAEngine> engine { new GBAEngine() };

  SongScene* songScene = new SongScene(engine);
  engine->setScene(songScene);

  while (true) {
    engine->update();
    engine->delay(1000);
  }

  return 0;
}
