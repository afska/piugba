#include "StartScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "player/PlaybackState.h"
#include "scenes/SongScene.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

StartScene::StartScene(std::shared_ptr<GBAEngine> engine) : Scene(engine) {}

std::vector<Background*> StartScene::backgrounds() {
  return {};
}

std::vector<Sprite*> StartScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void StartScene::load() {
  TextStream::instance().setText("piuGBA 0.0.1", 0, 0);
  TextStream::instance().setText(" con <3 para GameBoyCollectors", 1, 0);

  TextStream::instance().setText("SEL - Don't Bother Me (EASY)", 10, 0);

  TextStream::instance().setText("L - Beethoven Virus (HARD)", 12, 0);
  TextStream::instance().setText("R - Beethoven Virus (CRAZY)", 13, 0);

  TextStream::instance().setText("IZQ - Run to You (HARD)", 15, 0);
  TextStream::instance().setText("DER - Run to You (CRAZY)", 16, 0);

  TextStream::instance().setText("START - Extravaganza (CRAZY)", 18, 0);
}

void StartScene::tick(u16 keys) {
  if (keys & KEY_ANY && !engine->isTransitioning()) {
    const GBFS_FILE* fs = find_first_gbfs_file(0);

    char* name;
    u8 level;

    if (keys & KEY_SELECT) {
      name = (char*)"Don't Bother Me";
      level = 4;
    } else if (keys & KEY_L) {
      name = (char*)"Beethoven Virus";
      level = 7;
    } else if (keys & KEY_R) {
      name = (char*)"Beethoven Virus";
      level = 13;
    } else if (keys & KEY_LEFT) {
      name = (char*)"Run to You";
      level = 5;
    } else if (keys & KEY_RIGHT) {
      name = (char*)"Run to You";
      level = 12;
    } else if (keys & KEY_START) {
      name = (char*)"Extravaganza";
      level = 11;
    } else {
      return;
    }

    Song* song = Song_parse(fs, SongFile(name));
    Chart* chart = Song_findChartByLevel(song, level);

    engine->transitionIntoScene(new SongScene(engine, fs, song, chart),
                                new FadeOutScene(2));
  }
}

StartScene::~StartScene() {}
