#include "DanceGradeScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_grade.h"
#include "gameplay/models/Song.h"
#include "scenes/SelectionScene.h"
#include "utils/BackgroundUtils.h"

extern "C" {
#include "player/player.h"
}

const u32 TOTALS_Y[] = {37, 53, 69, 85, 101};
const u32 TOTAL_MAX_COMBO_Y = 117;
const u32 GRADE_X = 88;
const u32 GRADE_Y = 52;

DanceGradeScene::DanceGradeScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs,
                                 std::unique_ptr<Evaluation> evaluation)
    : Scene(engine) {
  this->fs = fs;
  this->evaluation = std::move(evaluation);
}

std::vector<Background*> DanceGradeScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> DanceGradeScene::sprites() {
  std::vector<Sprite*> sprites;

  for (u32 i = 0; i < totals.size(); i++)
    totals[i]->render(&sprites);
  maxComboTotal->render(&sprites);
  sprites.push_back(grade->get());

  return sprites;
}

void DanceGradeScene::load() {
  BACKGROUND_enable(false, false, false, false);
  grade = std::unique_ptr<Grade>{
      new Grade(evaluation->getGrade(), GRADE_X, GRADE_Y)};

  for (u32 i = 0; i < totals.size(); i++)
    totals[i] = std::unique_ptr<Total>{new Total(TOTALS_Y[i], i == 0)};
  maxComboTotal = std::unique_ptr<Total>{new Total(TOTAL_MAX_COMBO_Y, false)};

  totals[FeedbackType::PERFECT]->setValue(evaluation->perfects);
  totals[FeedbackType::GREAT]->setValue(evaluation->greats);
  totals[FeedbackType::GOOD]->setValue(evaluation->goods);
  totals[FeedbackType::BAD]->setValue(evaluation->bads);
  totals[FeedbackType::MISS]->setValue(evaluation->misses);
  maxComboTotal->setValue(evaluation->maxCombo);

  // TODO: Use points?

  setUpSpritesPalette();
  setUpBackground();
}

void DanceGradeScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(false, true, false, false);
    player_play(SOUND_RANK_A);
    hasStarted = true;
  }

  if (keys & KEY_ANY) {
    player_stop();
    engine->transitionIntoScene(new SelectionScene(engine),
                                new FadeOutScene(2));
  }
}

void DanceGradeScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_gradePal, sizeof(palette_gradePal)));
}

void DanceGradeScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_GRADE_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_GRADE_TILES, BG_GRADE_MAP, 1);
}
