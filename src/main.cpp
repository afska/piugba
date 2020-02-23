#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));
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

  player_forever([](char* src, char* src_pos, char* src_end) {
    unsigned int t = src_pos - src;
    t = fracumul(t, 1146880 * 1000);
    if (t > 5999000)
      t = 5999000;
    TextStream::instance().setText(std::to_string(t), 0, 0);
    TextStream::instance().setText(std::to_string(engine->getTimer()->getTotalMsecs()), 1, 0);

    engine->getTimer()->onvblank();
    engine->update();
  });

  return 0;
}
