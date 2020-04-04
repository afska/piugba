#include "Score.h"

const int LIFE_DIFFS[] = {2, 1, 0, -6, -12};
const int POINT_DIFFS[] = {1000, 500, 100, -200, -500};

const u32 ANIMATION_FRAMES = 3;
const u32 MIN_VISIBLE_COMBO = 4;

Score::Score(LifeBar* lifeBar) {
  this->lifeBar = lifeBar;
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};

  for (u32 i = 0; i < counters.size(); i++)
    counters[i] = 0;
}

void Score::update(FeedbackType feedbackType) {
  feedback->setType(feedbackType);
  feedback->show();

  updateCombo(feedbackType);
  updateLife(feedbackType);
  updateCounters(feedbackType);
  updatePoints(feedbackType);
}

void Score::tick() {
  feedback->tick();
  combo->tick();
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}

void Score::updateCombo(FeedbackType feedbackType) {
  u32 value = combo->getValue();

  switch (feedbackType) {
    case FeedbackType::MISS:
    case FeedbackType::BAD:
      value = 0;
      break;
    case FeedbackType::GREAT:
    case FeedbackType::PERFECT:
      value = value + 1;
      break;
    default:
      break;
  }

  if (value > maxCombo)
    maxCombo = value;

  combo->setValue(value);
  if (value >= MIN_VISIBLE_COMBO)
    combo->show();
}

void Score::updateLife(FeedbackType feedbackType) {
  life += clamp(LIFE_DIFFS[feedbackType], MIN_LIFE, MAX_LIFE + 1);
  lifeBar->setLife(life);
}

void Score::updateCounters(FeedbackType feedbackType) {
  counters[feedbackType]++;
}

void Score::updatePoints(FeedbackType feedbackType) {
  points += POINT_DIFFS[feedbackType];

  bool isPerfectOrGreat = feedbackType == FeedbackType::PERFECT ||
                          feedbackType == FeedbackType::GREAT;

  if (combo->getValue() > 50 && isPerfectOrGreat)
    points += 1000;
}
