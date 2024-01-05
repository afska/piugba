#include "DeathMixScene.h"

#include <string>

#include "SettingsScene.h"
#include "StartScene.h"  // TODO: REMOVE
#include "assets.h"
#include "data/content/_compiled_sprites/palette_selection.h"
#include "gameplay/Key.h"
#include "gameplay/SequenceMessages.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 DIFFICULTY_X = 111;
const u32 DIFFICULTY_Y = 115;
const u32 PROGRESS_X = 94;
const u32 PROGRESS_Y = 132;
const u32 BACK_X = 89;
const u32 BACK_Y = 114;
const u32 NEXT_X = 198;
const u32 NEXT_Y = 114;
const u32 MULTIPLIER_X = 143;
const u32 MULTIPLIER_Y = 95;

std::vector<Sprite*> DeathMixScene::sprites() {
  auto sprites = TalkScene::sprites();

  difficulty->render(&sprites);
  progress->render(&sprites);
  sprites.push_back(backButton->get());
  sprites.push_back(nextButton->get());
  sprites.push_back(multiplier->get());

  return sprites;
}

void DeathMixScene::load() {
  TalkScene::load();

  setUpSpritesPalette();

  multiplier = std::unique_ptr<Multiplier>{new Multiplier(
      MULTIPLIER_X, MULTIPLIER_Y, SAVEFILE_read8(SRAM->mods.multiplier))};
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
  settingsMenuInput = std::unique_ptr<InputHandler>{new InputHandler()};
}

void DeathMixScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  TalkScene::tick(keys);

  processKeys(keys);

  backButton->tick();
  nextButton->tick();
  multiplier->tick();

  processDifficultyChangeEvents();
  processMenuEvents();
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
    multiplier->setIsPressed(keys & KEY_SELECT);
    settingsMenuInput->setIsPressed(keys & KEY_START);
  } else {
    backButton->setIsPressed(KEY_UPLEFT(keys));
    nextButton->setIsPressed(KEY_UPRIGHT(keys));
    multiplier->setIsPressed(keys & KEY_SELECT);
    settingsMenuInput->setIsPressed(keys & KEY_START);
  }
}

void DeathMixScene::processDifficultyChangeEvents() {
  // TODO: IMPLEMENT
}

void DeathMixScene::processMenuEvents() {
  if (multiplier->hasBeenPressedNow()) {
    player_play(SOUND_MOD);
    SAVEFILE_write8(SRAM->mods.multiplier, multiplier->change());
  }

  if (settingsMenuInput->hasBeenPressedNow()) {
    player_stop();
    engine->transitionIntoScene(new SettingsScene(engine, fs, true),
                                new PixelTransitionEffect());
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
