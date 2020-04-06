#include <libgba-sprite-engine/gba_engine.h>

#include "scenes/StartScene.h"

extern "C" {
#include "player/player.h"
}

std::shared_ptr<GBAEngine> engine{new GBAEngine()};

int main() {
  player_init();
  engine->setScene(new StartScene(engine));
  player_forever([]() { engine->update(); });

  return 0;
}
