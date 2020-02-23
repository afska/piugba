#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));
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
  player_forever([](char* src, char* src_pos, char* src_end) {
    unsigned int msecs = src_pos - src;
    msecs = fracumul(msecs, 1146880 * 1000);
    songScene->setMsecs(msecs);

    engine->update();
  });

  return 0;
}
