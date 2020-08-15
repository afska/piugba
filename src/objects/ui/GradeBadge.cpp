#include "GradeBadge.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_grades_mini.h"
#include "utils/SpriteUtils.h"

GradeBadge::GradeBadge(u32 x, u32 y, bool reuseTiles) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_grades_miniTiles, sizeof(spr_grades_miniTiles))
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
