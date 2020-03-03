#include "Feedback.h"
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_feedback.h"

Feedback::Feedback(FeedbackType type) {
  this->type = type;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_feedbackTiles, sizeof(spr_feedbackTiles))
               .withSize(SIZE_64_32)
               .withLocation(16, 70)
               .buildPtr();
}

Sprite* Feedback::get() {
  return sprite.get();
}
