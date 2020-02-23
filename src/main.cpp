#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <iostream>
#include "scenes/SongScene.h"

extern "C" {
#include "player/gsm_main.h"
#include "utils/gbfs.h"
}

static std::shared_ptr<GBAEngine> engine{new GBAEngine()};

int main() {
  SongScene* songScene = new SongScene(engine);
  engine->setScene(songScene);

  auto fs = find_first_gbfs_file(0);
  auto betho = fs && gbfs_get_obj(fs, "beethoven-virus-full.gsm", NULL);
  TextStream::instance().setText(betho ? "Hay data" : "No hay data", 1, 1);

  player_init();
  engine->getTimer()->start();
  maino([]() { engine->update(); });

  return 0;
}
