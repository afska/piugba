#include "DeathMixScene.h"

#include <string>

#include "StartScene.h"  // TODO: REMOVE
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/SequenceMessages.h"
#include "utils/SceneUtils.h"

const u32 DIFFICULTY_X = 111;
const u32 DIFFICULTY_Y = 109;
const u32 PROGRESS_X = 94;
const u32 PROGRESS_Y = 126;
const u32 BACK_X = 89;
const u32 BACK_Y = 108;
const u32 NEXT_X = 198;
const u32 NEXT_Y = 108;

std::vector<Sprite*> DeathMixScene::sprites() {
  auto sprites = TalkScene::sprites();

  difficulty->render(&sprites);
  progress->render(&sprites);
  sprites.push_back(backButton->get());
  sprites.push_back(nextButton->get());

  return sprites;
}

void DeathMixScene::load() {
  TalkScene::load();

  setUpSpritesPalette();

  difficulty =
      std::unique_ptr<Difficulty>{new Difficulty(DIFFICULTY_X, DIFFICULTY_Y)};
  progress = std::unique_ptr<NumericProgress>{
      new NumericProgress(PROGRESS_X, PROGRESS_Y)};
  backButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::UPLEFT, false, true)};
  nextButton = std::unique_ptr<ArrowSelector>{
      new ArrowSelector(ArrowDirection::UPRIGHT, true, true)};
  backButton->get()->moveTo(BACK_X, BACK_Y);
  nextButton->get()->moveTo(NEXT_X, NEXT_Y);
}

void DeathMixScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TalkScene::tick(keys);

  processKeys(keys);

  backButton->tick();
  nextButton->tick();
}

void DeathMixScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>{new ForegroundPaletteManager(
          palette_selectionPal, sizeof(palette_selectionPal))};
}

void DeathMixScene::processKeys(u16 keys) {
  if (SAVEFILE_isUsingGBAStyle()) {
    backButton->setIsPressed(keys & KEY_L);
    nextButton->setIsPressed(keys & KEY_R);
    // settingsMenuInput->setIsPressed(keys & KEY_START);
  } else {
    backButton->setIsPressed(KEY_UPLEFT(keys));
    nextButton->setIsPressed(KEY_UPRIGHT(keys));
    // settingsMenuInput->setIsPressed(keys & KEY_START);
  }
}

void DeathMixScene::confirm(u16 keys) {
  bool isPressed =
      SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);

  if (isPressed) {
    engine->transitionIntoScene(new StartScene(engine, fs),
                                new PixelTransitionEffect());
  }
}
