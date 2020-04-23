#include "Difficulty.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_difficulties.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 8;
const u32 POSITION_Y = 100;

Difficulty::Difficulty() {
  SpriteBuilder<Sprite> builder1;
  SpriteBuilder<Sprite> builder2;

  leftSprite =
      builder1.withData(spr_difficultiesTiles, sizeof(spr_difficultiesTiles))
          .withSize(SIZE_64_32)
          .withLocation(POSITION_X, POSITION_Y)
          .buildPtr();

  rightSprite = builder2.withSize(SIZE_64_32)
                    .withLocation(POSITION_X, POSITION_Y)
                    .buildPtr();
  SPRITE_reuseTiles(rightSprite.get());
}

void Difficulty::setValue(DifficultyLevel value) {}

DifficultyLevel Difficulty::getValue() {
  return value;
}

void Difficulty::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(leftSprite.get());
  sprites->push_back(rightSprite.get());
}
