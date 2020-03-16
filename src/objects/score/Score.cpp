#include "Score.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>

const u32 ANIMATION_FRAMES = 3;

Score::Score() {
  feedback = std::unique_ptr<Feedback>{new Feedback()};
  combo = std::unique_ptr<Combo>{new Combo()};
}

void Score::update(FeedbackType feedbackType) {
  animationFrame = 0;
  feedback->get()->moveTo(FEEDBACK_POSITION_X,
                          FEEDBACK_POSITION_Y - ANIMATION_FRAMES);
  feedback->setType(feedbackType);
  combo->setValue(combo->getValue() + 1);
}

void Score::tick() {
  if (animationFrame < ANIMATION_FRAMES) {
    animationFrame++;
    feedback->get()->moveTo(
        FEEDBACK_POSITION_X,
        FEEDBACK_POSITION_Y - (ANIMATION_FRAMES - animationFrame));
  }
}

void Score::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(feedback->get());
  combo->render(sprites);
}
