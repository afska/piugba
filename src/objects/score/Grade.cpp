#include "Grade.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_grade_a.h"
#include "data/content/_compiled_sprites/spr_grade_b.h"
#include "data/content/_compiled_sprites/spr_grade_c.h"
#include "data/content/_compiled_sprites/spr_grade_d.h"
#include "data/content/_compiled_sprites/spr_grade_f.h"
#include "data/content/_compiled_sprites/spr_grade_s.h"
#include "utils/SpriteUtils.h"

const void* TILES[] = {spr_grade_sTiles, spr_grade_aTiles, spr_grade_bTiles,
                       spr_grade_cTiles, spr_grade_dTiles, spr_grade_fTiles};

Grade::Grade(GradeType type, u32 x, u32 y) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(TILES[type], sizeof(spr_grade_sTiles))
               .withSize(SIZE_64_64)
               .withLocation(x, y)
               .buildPtr();

  this->type = type;
}
