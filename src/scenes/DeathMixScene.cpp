#include "DeathMixScene.h"

#include <string>

#include "StartScene.h"  // TODO: REMOVE
#include "gameplay/Key.h"
#include "gameplay/SequenceMessages.h"
#include "utils/SceneUtils.h"

std::vector<Sprite*> DeathMixScene::sprites() {
  auto sprites = TalkScene::sprites();

  return sprites;
}

void DeathMixScene::load() {
  TalkScene::load();
}

void DeathMixScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TalkScene::tick(keys);
}

void DeathMixScene::confirm(u16 keys) {
  bool isPressed =
      SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);

  if (isPressed) {
    engine->transitionIntoScene(new StartScene(engine, fs),
                                new PixelTransitionEffect());
  }
}
