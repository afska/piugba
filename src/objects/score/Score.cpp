#include "Score.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>

Score::Score() {
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};
}

void Score::update(FeedbackType feedbackType) {
  feedback->setType(feedbackType);
  combo->setValue(combo->getValue() + 1);
}

void Score::tick() {
  // TODO: ANIMATE
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}
