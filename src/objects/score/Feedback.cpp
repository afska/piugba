#include "Feedback.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_feedback.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 16;
const u32 POSITION_Y = 60;

Feedback::Feedback() {
  type = FeedbackType::MISS;
  positionX = POSITION_X;
  positionY = POSITION_Y;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_feedbackTiles, sizeof(spr_feedbackTiles))
               .withSize(SIZE_64_32)
               .withLocation(positionX, positionY)
               .buildPtr();
}

void Feedback::setType(FeedbackType type) {
  this->type = type;
  SpriteUtils::goToFrame(sprite.get(), (int)type);
}

FeedbackType Feedback::getType() {
  return this->type;
}

Sprite* Feedback::get() {
  return sprite.get();
}
