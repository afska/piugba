#include "Score.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>

const u32 ANIMATION_FRAMES = 3;

Score::Score() {
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};
}

void Score::update(FeedbackType feedbackType) {
  feedback->setType(feedbackType);
  feedback->show();
  combo->setValue(combo->getValue() + 1);
}

void Score::tick() {
  feedback->tick();
  combo->tick();
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}
