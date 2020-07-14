#include "Instructor.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_instructors.h"
#include "utils/SpriteUtils.h"

Instructor::Instructor(InstructorType type, u32 x, u32 y) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_instructorsTiles, sizeof(spr_instructorsTiles))
               .withSize(SIZE_64_64)
               .withLocation(x, y)
               .buildPtr();

  SPRITE_goToFrame(sprite.get(), type);
}
