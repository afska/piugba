#include "DeathMixScene.h"

#include <string>

#include "StartScene.h"  // TODO: REMOVE
#include "data/content/_compiled_sprites/palette_deathmix.h"
#include "gameplay/Key.h"
#include "gameplay/SequenceMessages.h"
#include "utils/SceneUtils.h"

const u32 DIFFICULTY_X = 110;
const u32 DIFFICULTY_Y = 109;

std::vector<Sprite*> DeathMixScene::sprites() {
  auto sprites = TalkScene::sprites();

  difficulty->render(&sprites);
  // progress->render(&sprites);

  return sprites;
}

void DeathMixScene::load() {
  TalkScene::load();

  difficulty =
      std::unique_ptr<Difficulty>{new Difficulty(DIFFICULTY_X, DIFFICULTY_Y)};
  // progress = std::unique_ptr<NumericProgress>{new NumericProgress()};

  setUpSpritesPalette();
}

void DeathMixScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TalkScene::tick(keys);
}

void DeathMixScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>{new ForegroundPaletteManager(
          palette_deathmixPal, sizeof(palette_deathmixPal))};
}

void DeathMixScene::confirm(u16 keys) {
  bool isPressed =
      SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);

  if (isPressed) {
    engine->transitionIntoScene(new StartScene(engine, fs),
                                new PixelTransitionEffect());
  }
}
