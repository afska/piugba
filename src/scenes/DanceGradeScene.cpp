#include "DanceGradeScene.h"

#include "assets.h"
#include "data/content/_compiled_sprites/palette_grade.h"
#include "data/content/_compiled_sprites/palette_grade_multi.h"
#include "gameplay/Sequence.h"
#include "gameplay/multiplayer/Syncer.h"
#include "player/PlaybackState.h"
#include "utils/SceneUtils.h"
#include "utils/StringUtils.h"

extern "C" {
#include "player/player.h"
}

#define SCORE_TITLE "Score:"
#define PLAYER_1_WINS "Player 1 *WINS*"
#define PLAYER_2_WINS "Player 2 *WINS*"
#define PLAYER_1_ARROW "<<<"
#define PLAYER_2_ARROW ">>>"

const u32 ID_MAIN_BACKGROUND = 1;
const u32 BANK_BACKGROUND_TILES = 0;
const u32 BANK_BACKGROUND_MAP = 16;
const u32 TEXT_COLOR = 0x7FFF;
const u32 TEXT_ROW = 17;
const u32 SCORE_DIGITS = 8;

const u32 TOTALS_X[] = {11, 160};
const u32 TOTALS_Y[] = {37, 53, 69, 85, 101};
const u32 TOTAL_MAX_COMBO_Y = 117;
const u32 GRADE_X = 88;
const u32 GRADE_Y = 52;
const u32 MINI_GRADE_X[] = {37, 187};
const u32 MINI_GRADE_Y = 137;

DanceGradeScene::DanceGradeScene(std::shared_ptr<GBAEngine> engine,
                                 const GBFS_FILE* fs,
                                 std::unique_ptr<Evaluation> evaluation,
                                 std::unique_ptr<Evaluation> remoteEvaluation,
                                 bool differentCharts,
                                 bool isLastSong)
    : Scene(engine) {
  this->fs = fs;
  this->evaluation = std::move(evaluation);
  this->remoteEvaluation = std::move(remoteEvaluation);
  this->differentCharts = differentCharts;
  this->isLastSong = isLastSong;
}

std::vector<Background*> DanceGradeScene::backgrounds() {
  return {bg.get()};
}

std::vector<Sprite*> DanceGradeScene::sprites() {
  std::vector<Sprite*> sprites;

  for (auto& it : totals)
    it->render(&sprites);

  if (isVs())
    for (auto& it : remoteTotals)
      it->render(&sprites);

  maxComboTotal->render(&sprites);

  if (isVs())
    remoteMaxComboTotal->render(&sprites);

  if (isVs())
    for (auto& it : miniGrades)
      sprites.push_back(it->get());
  else
    sprites.push_back(grade->get());

  return sprites;
}

void DanceGradeScene::load() {
  if (isMultiplayer())
    syncer->clearTimeout();

  SCENE_init();
  TextStream::instance().setMosaic(true);

  setUpSpritesPalette();
  setUpBackground();
  printScore();

  u32 totalsX = TOTALS_X[!isVs() || syncer->getLocalPlayerId() == 1];
  for (u32 i = 0; i < totals.size(); i++)
    totals[i] = std::unique_ptr<Total>{new Total(totalsX, TOTALS_Y[i], i == 0)};
  maxComboTotal =
      std::unique_ptr<Total>{new Total(totalsX, TOTAL_MAX_COMBO_Y, false)};

  totals[FeedbackType::PERFECT]->setValue(evaluation->perfects);
  totals[FeedbackType::GREAT]->setValue(evaluation->greats);
  totals[FeedbackType::GOOD]->setValue(evaluation->goods);
  totals[FeedbackType::BAD]->setValue(evaluation->bads);
  totals[FeedbackType::MISS]->setValue(evaluation->misses);
  maxComboTotal->setValue(evaluation->maxCombo);

  if (isVs()) {
    u8 localId = syncer->getLocalPlayerId();
    u8 remoteId = syncer->getRemotePlayerId();

    miniGrades[0] = std::unique_ptr<GradeBadge>{
        new GradeBadge(MINI_GRADE_X[localId], MINI_GRADE_Y, false, true)};
    miniGrades[1] = std::unique_ptr<GradeBadge>{
        new GradeBadge(MINI_GRADE_X[remoteId], MINI_GRADE_Y, true, true)};
    miniGrades[0]->setType(evaluation->getGrade());
    miniGrades[1]->setType(remoteEvaluation->getGrade());

    for (u32 i = 0; i < remoteTotals.size(); i++)
      remoteTotals[i] = std::unique_ptr<Total>{
          new Total(TOTALS_X[remoteId], TOTALS_Y[i], false)};
    remoteMaxComboTotal = std::unique_ptr<Total>{
        new Total(TOTALS_X[remoteId], TOTAL_MAX_COMBO_Y, false)};

    remoteTotals[FeedbackType::PERFECT]->setValue(remoteEvaluation->perfects);
    remoteTotals[FeedbackType::GREAT]->setValue(remoteEvaluation->greats);
    remoteTotals[FeedbackType::GOOD]->setValue(remoteEvaluation->goods);
    remoteTotals[FeedbackType::BAD]->setValue(remoteEvaluation->bads);
    remoteTotals[FeedbackType::MISS]->setValue(remoteEvaluation->misses);
    remoteMaxComboTotal->setValue(remoteEvaluation->maxCombo);
  } else
    grade = std::unique_ptr<Grade>{
        new Grade(evaluation->getGrade(), GRADE_X, GRADE_Y)};
}

void DanceGradeScene::tick(u16 keys) {
  if (engine->isTransitioning() || !hasStarted)
    return;

  if (SEQUENCE_isMultiplayerSessionDead()) {
    player_stop();
    SEQUENCE_goToMultiplayerGameMode(SAVEFILE_getGameMode());
    return;
  }

  if (isMultiplayer()) {
    processMultiplayerUpdates();
    if (!syncer->isPlaying())
      return;
  }

  if (PlaybackState.hasFinished && (keys & KEY_ANY)) {
    if (isMultiplayer()) {
      if (syncer->isMaster())
        syncer->send(SYNC_EVENT_CONFIRM_SONG_END, 0);
      else
        return;
    }

    finish();
  }
}

void DanceGradeScene::render() {
  if (engine->isTransitioning())
    return;

  if (!hasStarted) {
    BACKGROUND_enable(true, true, false, false);
    SPRITE_enable();
    playSound();
    hasStarted = true;
  }
}

void DanceGradeScene::setUpSpritesPalette() {
  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>{new ForegroundPaletteManager(
          isVs() ? palette_grade_multiPal : palette_gradePal,
          isVs() ? sizeof(palette_grade_multiPal) : sizeof(palette_gradePal))};
}

void DanceGradeScene::setUpBackground() {
  if (isVs()) {
    backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_GRADE_MULTI_PALETTE);
    bg = BACKGROUND_loadBackgroundFiles(fs, BG_GRADE_MULTI_TILES,
                                        BG_GRADE_MULTI_MAP, ID_MAIN_BACKGROUND);
  } else {
    backgroundPalette = BACKGROUND_loadPaletteFile(fs, BG_GRADE_PALETTE);
    bg = BACKGROUND_loadBackgroundFiles(fs, BG_GRADE_TILES, BG_GRADE_MAP,
                                        ID_MAIN_BACKGROUND);
  }
  bg->useCharBlock(BANK_BACKGROUND_TILES);
  bg->useMapScreenBlock(BANK_BACKGROUND_MAP);
  bg->setMosaic(true);
}

void DanceGradeScene::finish() {
  player_stop();
  SEQUENCE_goToWinOrSelection(isLastSong);
}

void DanceGradeScene::printScore() {
  TextStream::instance().setFontColor(TEXT_COLOR);

  if (isVs()) {
    TextStream::instance().scrollNow(0, -1);

    auto player1Evaluation = syncer->getLocalPlayerId() == 0
                                 ? evaluation.get()
                                 : remoteEvaluation.get();
    auto player2Evaluation = syncer->getLocalPlayerId() == 1
                                 ? evaluation.get()
                                 : remoteEvaluation.get();

    auto player1Points = getMultiplayerPointsOf(player1Evaluation);
    auto player2Points = getMultiplayerPointsOf(player2Evaluation);

    if (player1Points >= player2Points) {
      SCENE_write(PLAYER_1_WINS, TEXT_ROW);
      SCENE_write(PLAYER_1_ARROW, TEXT_ROW + 1);
    } else {
      SCENE_write(PLAYER_2_WINS, TEXT_ROW);
      SCENE_write(PLAYER_2_ARROW, TEXT_ROW + 1);
    }

    if (differentCharts) {
      auto points1Str = std::to_string(player1Points);
      STRING_padLeft(points1Str, 3, '0');
      auto points2Str = std::to_string(player2Points);
      STRING_padLeft(points2Str, 3, '0');
      STRING_padLeft(points2Str, TEXT_TOTAL_COLS - points1Str.length());
      auto pointsStr = points1Str + points2Str;
      SCENE_write(pointsStr, 0);
    } else {
      auto points1Str = pointsToString(player1Points);
      auto points2Str = pointsToString(player2Points);
      STRING_padLeft(points2Str, TEXT_TOTAL_COLS - points1Str.length());
      auto pointsStr = points1Str + points2Str;
      SCENE_write(pointsStr, 0);
    }
  } else {
    SCENE_write(SCORE_TITLE, TEXT_ROW);
    SCENE_write(pointsToString(evaluation->points), TEXT_ROW + 1);
  }
}

std::string DanceGradeScene::pointsToString(u32 points) {
  auto pointsStr = std::to_string(points);
  STRING_padLeft(pointsStr, SCORE_DIGITS, '0');
  return pointsStr;
}

u32 DanceGradeScene::getMultiplayerPointsOf(Evaluation* evaluation) {
  return differentCharts ? evaluation->percent : evaluation->points;
}

void DanceGradeScene::playSound() {
  auto gradeType = isVs() ? miniGrades[0].get()->getType() : grade->getType();

  switch (gradeType) {
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

void DanceGradeScene::processMultiplayerUpdates() {
  auto remoteId = syncer->getRemotePlayerId();

  while (syncer->isPlaying() && linkUniversal->canRead(remoteId)) {
    u16 message = linkUniversal->read(remoteId);
    u8 event = SYNC_MSG_EVENT(message);

    if (syncer->isMaster()) {
      syncer->registerTimeout();
      continue;
    }

    switch (event) {
      case SYNC_EVENT_CONFIRM_SONG_END: {
        finish();

        syncer->clearTimeout();
        break;
      }
      default: {
        syncer->registerTimeout();
      }
    }
  }
}
