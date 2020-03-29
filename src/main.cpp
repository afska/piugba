#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"

extern "C" {
#include "player/player.h"
#include "utils/gbfs/gbfs.h"
}

char* EXAMPLE_PIUS = (char*)"TestSong.pius";
char* EXAMPLE_GSM = (char*)"TestSong.gsm";
u8 EXAMPLE_LEVEL = 7;

std::shared_ptr<GBAEngine> engine{new GBAEngine()};
SongScene* songScene;

int main() {
  const GBFS_FILE* fs = find_first_gbfs_file(0);
  Song* song = Song_parse(fs, EXAMPLE_PIUS);
  Chart* chart = Song_findChartByLevel(song, EXAMPLE_LEVEL);

  songScene = new SongScene(engine, chart);
  engine->setScene(songScene);

  player_init();
  player_play(EXAMPLE_GSM);
  player_forever([](u32 msecs) {
    songScene->msecs = msecs;

    engine->update();
  });

  Song_free(song);  // TODO: When?

  return 0;
}
