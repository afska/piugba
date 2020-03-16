#include "Feedback.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_feedback.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 16;
const u32 POSITION_Y = 60;

Feedback::Feedback() {
  this->type = FeedbackType::MISS;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_feedbackTiles, sizeof(spr_feedbackTiles))
               .withSize(SIZE_64_32)
               .withLocation(POSITION_X, POSITION_Y)
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
  sprite->moveTo(POSITION_X, POSITION_Y);
}

void Feedback::hide() {
  SpriteUtils::hide(sprite.get());
}

Sprite* Feedback::get() {
  return sprite.get();
}
