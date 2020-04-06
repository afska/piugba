#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>

#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"

extern "C" {
#include "player/player.h"
#include "utils/gbfs/gbfs.h"
}

char* SONG_NAME = (char*)"TestSong";
u8 EXAMPLE_LEVEL = 7;

std::shared_ptr<GBAEngine> engine{new GBAEngine()};
SongScene* songScene;

int main() {
  const GBFS_FILE* fs = find_first_gbfs_file(0);
  Song* song = Song_parse(fs, SongFile(SONG_NAME));
  Chart* chart = Song_findChartByLevel(song, EXAMPLE_LEVEL);

  player_init();
  songScene = new SongScene(engine, fs, song, chart);
  engine->setScene(songScene);
  player_forever([](u32 msecs) {
    songScene->msecs = msecs;

    engine->update();
  });

  return 0;
}
