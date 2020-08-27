#include "DanceGradeScene.h"

#include <libgba-sprite-engine/effects/fade_out_scene.h>

#include "assets.h"
#include "data/content/_compiled_sprites/palette_grade.h"
#include "gameplay/Sequence.h"
#include "player/PlaybackState.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

#define SCORE_TITLE "Score:"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_ROW = 17;
const u32 SCORE_DIGITS = 8;

const u32 TOTALS_Y[] = {37, 53, 69, 85, 101};
const u32 TOTAL_MAX_COMBO_Y = 117;
const u32 GRADE_X = 88;
const u32 GRADE_Y = 52;

DanceGradeScene::DanceGradeScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs,
                                 std::unique_ptr<Evaluation> evaluation,
                                 bool isLastSong)
    : Scene(engine) {
  this->fs = fs;
  this->evaluation = std::move(evaluation);
  this->isLastSong = isLastSong;
}

std::vector<Background*> DanceGradeScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> DanceGradeScene::sprites() {
  std::vector<Sprite*> sprites;

  for (auto& it : totals)
    it->render(&sprites);
  maxComboTotal->render(&sprites);
  sprites.push_back(grade->get());

  return sprites;
}

void DanceGradeScene::load() {
  SCENE_init();

  setUpSpritesPalette();
  setUpBackground();
  printScore();

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
}

void DanceGradeScene::tick(u16 keys) {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    hasStarted = true;
    playSound();
  }

  if (PlaybackState.hasFinished && (keys & KEY_ANY)) {
    player_stop();
    SEQUENCE_goToWinOrSelection(isLastSong);
  }
}

void DanceGradeScene::setUpSpritesPalette() {
  foregroundPalette = std::unique_ptr<ForegroundPaletteManager>(
      new ForegroundPaletteManager(palette_gradePal, sizeof(palette_gradePal)));
}

void DanceGradeScene::setUpBackground() {
  backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_GRADE_PALETTE);
  bg = BACKGROUND_loadBackgroundFiles(fs, BG_GRADE_TILES, BG_GRADE_MAP,
                                      ID_MAIN_BACKGROUND);
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
}

void DanceGradeScene::printScore() {
  TextStream::instance().setFontColor(TEXT_COLOR);

  auto points = std::to_string(evaluation->points);
  while (points.length() < SCORE_DIGITS)
    points = "0" + points;

  SCENE_write(SCORE_TITLE, TEXT_ROW);
  SCENE_write(points, TEXT_ROW + 1);
}

void DanceGradeScene::playSound() {
  switch (grade->getType()) {
    case GradeType::S: {
      player_play(SOUND_RANK_S);
      break;
    }
    case GradeType::A: {
      player_play(SOUND_RANK_A);
      break;
    }
    case GradeType::B: {
      player_play(SOUND_RANK_B);
      break;
    }
    case GradeType::C: {
      player_play(SOUND_RANK_C);
      break;
    }
    case GradeType::D: {
      player_play(SOUND_RANK_D);
      break;
    }
    case GradeType::F: {
      player_play(SOUND_RANK_F);
      break;
    }
    default: {
    }
  }
}
