#include "Score.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>

Score::Score() {
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};
  digit1 = std::unique_ptr<ComboDigit>{new ComboDigit(0)};
  digit2 = std::unique_ptr<ComboDigit>{new ComboDigit(1)};
  digit3 = std::unique_ptr<ComboDigit>{new ComboDigit(2)};
}

void Score::update(FeedbackType feedbackType) {
  feedback->set(feedbackType);
}

void Score::tick() {
  // TODO: ANIMATE
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  sprites->push_back(combo->get());
  sprites->push_back(digit1->get());
  sprites->push_back(digit2->get());
  sprites->push_back(digit3->get());
}
