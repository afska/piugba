#include "Score.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

const int LIFE_DIFFS[] = {1, 1, 0, -6, -12};
const int POINT_DIFFS[] = {1000, 500, 100, -200, -500};

const u32 ANIMATION_FRAMES = 3;
const u32 MIN_VISIBLE_COMBO = 4;

Score::Score(LifeBar* lifeBar, u8 playerId) {
  lifeBar->setLife(life);
  this->lifeBar = lifeBar;
  this->playerId = playerId;

  feedback = std::unique_ptr<Feedback>{new Feedback(playerId)};
  combo = std::unique_ptr<Combo>{new Combo(playerId)};

  for (u32 i = 0; i < counters.size(); i++)
    counters[i] = 0;
}

bool Score::update(FeedbackType feedbackType, bool isLong) {
  if (isLong)
    longNotes++;

  feedback->setType(feedbackType);
  feedback->show();

  bool isAlive = updateLife(feedbackType);
  updateCombo(feedbackType);
  updateCounters(feedbackType);
  updatePoints(feedbackType);

  return isAlive;
}

std::unique_ptr<Evaluation> Score::evaluate() {
  auto evaluation = std::unique_ptr<Evaluation>{new Evaluation()};
  evaluation->perfects = counters[FeedbackType::PERFECT];
  evaluation->greats = counters[FeedbackType::GREAT];
  evaluation->goods = counters[FeedbackType::GOOD];
  evaluation->bads = counters[FeedbackType::BAD];
  evaluation->misses = counters[FeedbackType::MISS];
  evaluation->maxCombo = maxCombo;
  evaluation->points = points;
  evaluation->longNotes = longNotes;

  return evaluation;
}

void Score::relocate() {
  feedback->relocate();
  combo->relocate();
}

void Score::tick() {
  feedback->tick();
  combo->tick();
}

bool Score::updateLife(FeedbackType feedbackType) {
  if (lifeBar->getIsDead())
    return true;

  u32 bonus = 0;
  if (feedbackType == FeedbackType::PERFECT) {
    halfLifeBonus = !halfLifeBonus;
    if (halfLifeBonus)
      bonus = 1;
  }

  life = min(life + LIFE_DIFFS[feedbackType] + bonus, MAX_LIFE);
  bool isAlive = life >= MIN_LIFE;
  if (!isAlive)
    life = MIN_LIFE;
  lifeBar->setLife(life);

  return isAlive;
}

void Score::updateCombo(FeedbackType feedbackType) {
  u32 value = combo->getValue();

  switch (feedbackType) {
    case FeedbackType::MISS:
      if (hasMissCombo)
        value++;
      else {
        value = 1;
        hasMissCombo = true;
      }
      break;
    case FeedbackType::BAD:
      hasMissCombo = false;
      value = 0;
      break;
    case FeedbackType::GOOD:
      if (hasMissCombo) {
        hasMissCombo = false;
        value = 0;
      }
      break;
    case FeedbackType::GREAT:
    case FeedbackType::PERFECT:
      if (hasMissCombo) {
        hasMissCombo = false;
        value = 0;
      }

      value = value + 1;
      break;
    default:
      break;
  }

  if (!hasMissCombo && value > maxCombo)
    maxCombo = value;

  combo->setValue(value * (hasMissCombo ? -1 : 1));
  if (value >= MIN_VISIBLE_COMBO)
    combo->show();
  else
    combo->hide();
}

void Score::updateCounters(FeedbackType feedbackType) {
  counters[feedbackType]++;
}

void Score::updatePoints(FeedbackType feedbackType) {
  if (lifeBar->getIsDead())
    return;

  points = max(points + POINT_DIFFS[feedbackType], 0);

  bool isPerfectOrGreat = feedbackType == FeedbackType::PERFECT ||
                          feedbackType == FeedbackType::GREAT;

  if (combo->getValue() > 50 && isPerfectOrGreat)
    points += 1000;
}
