#include "Difficulty.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_difficulties.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 79;
const u32 POSITION_Y = 16;
const u32 FRAME_NOR = 0;
const u32 FRAME_MAL = 1;
const u32 FRAME_HARD = 2;
const u32 FRAME_CRAZY = 3;
const u32 OFFSET_X_MAL = 40;
const u32 OFFSET_X_HARD = 14;
const u32 OFFSET_X_CRAZY = 10;

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
  SPRITE_goToFrame(rightSprite.get(), FRAME_MAL);

  setValue(value);
}

void Difficulty::setValue(DifficultyLevel value) {
  this->value = value;

  switch (value) {
    case DifficultyLevel::NORMAL:
      SPRITE_goToFrame(leftSprite.get(), FRAME_NOR);
      leftSprite->moveTo(POSITION_X, POSITION_Y);
      rightSprite->moveTo(POSITION_X + OFFSET_X_MAL, POSITION_Y);
      break;
    case DifficultyLevel::HARD:
      SPRITE_goToFrame(leftSprite.get(), FRAME_HARD);
      leftSprite->moveTo(POSITION_X + OFFSET_X_HARD, POSITION_Y);
      SPRITE_hide(rightSprite.get());
      break;
    case DifficultyLevel::CRAZY:
      SPRITE_goToFrame(leftSprite.get(), FRAME_CRAZY);
      leftSprite->moveTo(POSITION_X + OFFSET_X_CRAZY, POSITION_Y);
      SPRITE_hide(rightSprite.get());
      break;
    default:
      SPRITE_hide(leftSprite.get());
      SPRITE_hide(rightSprite.get());
      break;
  }
}

void Difficulty::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(leftSprite.get());
  sprites->push_back(rightSprite.get());
}
