#include "GradeBadge.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_grades_mini.h"
#include "data/content/_compiled_sprites/spr_grades_mini_evaluation.h"
#include "utils/SpriteUtils.h"

#define TILES_SELECTION spr_grades_miniTiles
#define TILES_EVALUATION spr_grades_mini_evaluationTiles

GradeBadge::GradeBadge(u32 x, u32 y, bool reuseTiles, bool isEvaluation) {
  SpriteBuilder<Sprite> builder;
  sprite = builder
               .withData(isEvaluation ? TILES_EVALUATION : TILES_SELECTION,
                         sizeof(spr_grades_miniTiles))
               .withSize(SIZE_16_16)
               .withLocation(x, y)
               .buildPtr();

  this->x = x;
  this->y = y;

  SPRITE_goToFrame(sprite.get(), 0);

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());
}

void GradeBadge::setType(GradeType type) {
  if (type == GradeType::UNPLAYED)
    SPRITE_hide(sprite.get());
  else {
    SPRITE_goToFrame(sprite.get(), type);
    sprite->moveTo(x, y);
  }
}
