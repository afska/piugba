#include "StageBreakScene.h"

#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "gameplay/models/Song.h"
#include "scenes/StartScene.h"

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
  log_text("Hey! Why don't you just get up and dance man?");
}

void StageBreakScene::tick(u16 keys) {
  if (keys & KEY_ANY && !engine->isTransitioning()) {
    engine->transitionIntoScene(new StartScene(engine), new FadeOutScene(2));
  }
}

StageBreakScene::~StageBreakScene() {}
