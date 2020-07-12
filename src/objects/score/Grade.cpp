#include "Grade.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_grades.h"
#include "utils/SpriteUtils.h"

Grade::Grade(GradeType type, u32 x, u32 y) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_gradesTiles, sizeof(spr_gradesTiles))
               .withSize(SIZE_64_64)
               .withLocation(x, y)
               .buildPtr();

  SPRITE_goToFrame(sprite.get(), type);

  this->type = type;
}
