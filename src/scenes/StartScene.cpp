#include "StartScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "player/PlaybackState.h"
#include "scenes/SongScene.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

char* SONG_NAME = (char*)"Extravaganza";
u8 EXAMPLE_LEVEL = 11;
// char* SONG_NAME = (char*)"Beethoven Virus";
// u8 EXAMPLE_LEVEL = 13;

StartScene::StartScene(std::shared_ptr<GBAEngine> engine) : Scene(engine) {}

std::vector<Background*> StartScene::backgrounds() {
  return {};
}

std::vector<Sprite*> StartScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void StartScene::load() {
  log_text("Press any key to play");
}

void StartScene::tick(u16 keys) {
  if (keys & KEY_ANY && !engine->isTransitioning()) {
    const GBFS_FILE* fs = find_first_gbfs_file(0);
    Song* song = Song_parse(fs, SongFile(SONG_NAME));
    Chart* chart = Song_findChartByLevel(song, EXAMPLE_LEVEL);

    engine->transitionIntoScene(new SongScene(engine, fs, song, chart),
                                new FadeOutScene(2));
  }
}

StartScene::~StartScene() {}
