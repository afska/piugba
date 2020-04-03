#include "Score.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

const u32 ANIMATION_FRAMES = 3;

Score::Score() {
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};

  for (int i = 0; i < counters.size(); i++)
    counters[i] = 0;
}

void Score::update(FeedbackType feedbackType) {
  feedback->setType(feedbackType);
  feedback->show();
  combo->setValue(feedbackType == FeedbackType::MISS ? 0
                                                     : combo->getValue() + 1);
  combo->show();
}

void Score::tick() {
  feedback->tick();
  combo->tick();
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}
