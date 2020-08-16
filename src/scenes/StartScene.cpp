#include "StartScene.h"

#include "assets.h"
#include "data/content/_compiled_sprites/palette_controls.h"  // TODO: CAMBIAR
#include "gameplay/Key.h"
#include "player/PlaybackState.h"
#include "scenes/SelectionScene.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/fxes.h"
#include "player/player.h"
}

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 20;
const u32 PIXEL_BLINK_LEVEL = 4;

StartScene::StartScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> StartScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> StartScene::sprites() {
  std::vector<Sprite*> sprites;

  // sprites.push_back(buttons[ArrowDirection::UPLEFT]->get());
  // sprites.push_back(buttons[ArrowDirection::DOWNLEFT]->get());
  // sprites.push_back(buttons[ArrowDirection::CENTER]->get());
  // sprites.push_back(buttons[ArrowDirection::UPRIGHT]->get());
  // sprites.push_back(buttons[ArrowDirection::DOWNRIGHT]->get());

  return sprites;
}

void StartScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  pixelBlink = std::unique_ptr<PixelBlink>(new PixelBlink(PIXEL_BLINK_LEVEL));

  setUpArrows();
}

void StartScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
    player_play(SOUND_LOOP);
  }

  processKeys(keys);

  if (PlaybackState.hasFinished)
    player_play(SOUND_LOOP);

  pixelBlink->tick();
}

void StartScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          palette_controlsPal, sizeof(palette_controlsPal)));
}

void StartScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_START_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_START_TILES, BG_START_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void StartScene::setUpArrows() {
  // for (u32 i = 0; i < ARROWS_TOTAL; i++) {
  //   auto direction = static_cast<ArrowDirection>(i);
  //   buttons.push_back(std::unique_ptr<ArrowSelector>{new ArrowSelector(
  //       direction, direction != ArrowDirection::UPLEFT, true)});
  // }

  // buttons.push_back(std::unique_ptr<ArrowSelector>{new ArrowSelector(
  //     static_cast<ArrowDirection>(ArrowDirection::CENTER), true, true)});
}

void StartScene::processKeys(u16 keys) {
  // buttons[ArrowDirection::DOWNLEFT]->setIsPressed(KEY_DOWNLEFT(keys));
  // buttons[ArrowDirection::UPLEFT]->setIsPressed(KEY_UPLEFT(keys));
  // buttons[ArrowDirection::CENTER]->setIsPressed(KEY_CENTER(keys));
  // buttons[ArrowDirection::UPRIGHT]->setIsPressed(KEY_UPRIGHT(keys));
  // buttons[ArrowDirection::DOWNRIGHT]->setIsPressed(KEY_DOWNRIGHT(keys));
}
