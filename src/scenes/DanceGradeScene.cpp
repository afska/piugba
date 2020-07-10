#include "DanceGradeScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_grade.h"
#include "gameplay/models/Song.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"

DanceGradeScene::DanceGradeScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs)
    : Scene(engine) {
  this->fs = fs;
}

std::vector<Background*> DanceGradeScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> DanceGradeScene::sprites() {
  std::vector<Sprite*> sprites;

  sprites.push_back(grade->get());

  return sprites;
}

void DanceGradeScene::load() {
  BACKGROUND_enable(false, true, false, false);
  grade = std::unique_ptr<Grade>{new Grade(GradeType::S, 50, 50)};

  setUpSpritesPalette();
  setUpBackground();
}

void DanceGradeScene::tick(u16 keys) {
  if (keys & KEY_ANY && !engine->isTransitioning()) {
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
  }
}

void DanceGradeScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_gradePal, sizeof(palette_gradePal)));
}

void DanceGradeScene::setUpBackground() {
  u32 backgroundPaletteLength;
  auto backgroundPaletteData =
      (COLOR*)gbfs_get_obj(fs, BG_GRADE_PALETTE, &backgroundPaletteLength);

  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          backgroundPaletteData, backgroundPaletteLength));

  u32 backgroundTilesLength, backgroundMapLength;
  auto backgroundTilesData =
      gbfs_get_obj(fs, BG_GRADE_TILES, &backgroundTilesLength);
  auto backgroundMapData = gbfs_get_obj(fs, BG_GRADE_MAP, &backgroundMapLength);

  bg = std::unique_ptr<Background>(
      new Background(1, backgroundTilesData, backgroundTilesLength,
                     backgroundMapData, backgroundMapLength));
}
