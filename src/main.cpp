#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
#include "player/player.h"
#include "utils/gbfs/gbfs.h"
}

std::shared_ptr<GBAEngine> engine{new GBAEngine()};
SongScene* songScene;

int main() {
  const GBFS_FILE* fs = find_first_gbfs_file(0);

  songScene = new SongScene(engine, fs);
  engine->setScene(songScene);

  player_init();
  player_play((char*)"beethoven-virus.gsm");
  player_forever([](u32 msecs) {
    songScene->msecs = msecs;

    engine->update();
  });

  return 0;
}
