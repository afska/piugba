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
               .withLocation(16, 60)
               .buildPtr();
}

void Feedback::set(FeedbackType type) {
  this->type = type;
  SpriteUtils::goToFrame(sprite.get(), (int) type);
}

Sprite* Feedback::get() {
  return sprite.get();
}
