#include "Feedback.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_feedback.h"
#include "utils/SpriteUtils.h"

Feedback::Feedback() {
  this->type = FeedbackType::MISS;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_feedbackTiles, sizeof(spr_feedbackTiles))
               .withSize(SIZE_64_32)
               .withLocation(FEEDBACK_POSITION_X, FEEDBACK_POSITION_Y)
               .buildPtr();
}

void Feedback::setType(FeedbackType type) {
  this->type = type;
  SpriteUtils::goToFrame(sprite.get(), (int)type);
}

FeedbackType Feedback::getType() {
  return this->type;
}

void Feedback::show() {
  sprite->moveTo(FEEDBACK_POSITION_X, FEEDBACK_POSITION_Y - 3);
}

void Feedback::hide() {
  SpriteUtils::hide(sprite.get());
}

Sprite* Feedback::get() {
  return sprite.get();
}
