#include "StageBreakScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/SelectionScene.h"

StageBreakScene::StageBreakScene(std::shared_ptr<GBAEngine> engine)
    : Scene(engine) {}

std::vector<Background*> StageBreakScene::backgrounds() {
  return {};
}

std::vector<Sprite*> StageBreakScene::sprites() {
  std::vector<Sprite*> sprites;

  return sprites;
}

void StageBreakScene::load() {
  TextStream::instance().setText("Hey!", 7, 0);
  TextStream::instance().setText("Why don't you", 9, 0);
  TextStream::instance().setText("just get up and dance,", 10, 7);
  TextStream::instance().setText("man?", 11, 26);
}

void StageBreakScene::tick(u16 keys) {
  if (keys & KEY_ANY && !engine->isTransitioning()) {
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
  }
}

StageBreakScene::~StageBreakScene() {}
