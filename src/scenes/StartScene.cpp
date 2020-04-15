#include "StartScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/SongScene.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

static std::vector<std::string> songFileNames;

bool stringEndsWith(const char* str, const char* suffix) {
  if (!str || !suffix)
    return false;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return false;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
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
  TextStream::instance().setText("piuGBA 0.0.3", 0, 0);
  TextStream::instance().setText(" con <3 para GameBoyCollectors", 1, 0);

  // const GBFS_FILE* fs = find_first_gbfs_file(0);
  // u32 count = gbfs_count_objs(fs);
  // for (u32 i = 0; i < count; i++) {
  //   char name[24];
  //   gbfs_get_nth_obj(fs, i, name, NULL);
  //   if (stringEndsWith(name, ".pius")) {
  //     auto fileName = std::string(name);
  //     fileName = fileName.replace(fileName.length() - 5, 5, "");
  //     songFileNames.push_back(fileName);
  //   }
  // }

  // u32 row = 6;
  // for (auto& fileName : songFileNames) {
  //   TextStream::instance().setText(fileName, row, 0);
  //   row++;
  // }

  TextStream::instance().setText("SEL - Don't Bother Me (HARD)", 6, 0);
  TextStream::instance().setText("ARR - Witch Doctor (CRAZY)", 8, 0);
  TextStream::instance().setText("B - Beat of the War 2 (16)", 9, 0);
  TextStream::instance().setText("A - Tepris (CRAZY)", 10, 0);
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
      level = 6;
    } else if (keys & KEY_UP) {
      name = (char*)"Witch Doctor";
      level = 16;
    } else if (keys & KEY_B) {
      name = (char*)"Beat of the War 2";
      level = 16;
    } else if (keys & KEY_A) {
      name = (char*)"Tepris";
      level = 16;
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
