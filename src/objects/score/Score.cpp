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

bool Score::update(FeedbackType feedbackType) {
  feedback->setType(feedbackType);
  feedback->show();

  bool isAlive = updateLife(feedbackType);
  updateCombo(feedbackType);
  updateCounters(feedbackType);
  updatePoints(feedbackType);

  return isAlive;
}

void Score::tick() {
  feedback->tick();
  combo->tick();
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}

bool Score::updateLife(FeedbackType feedbackType) {
  life = min(life + LIFE_DIFFS[feedbackType], MAX_LIFE);
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
  points += POINT_DIFFS[feedbackType];

  bool isPerfectOrGreat = feedbackType == FeedbackType::PERFECT ||
                          feedbackType == FeedbackType::GREAT;

  if (combo->getValue() > 50 && isPerfectOrGreat)
    points += 1000;
}
