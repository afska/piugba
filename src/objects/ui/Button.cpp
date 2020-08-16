#include "Button.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_buttons.h"
#include "utils/SpriteUtils.h"

const u32 SELECTED_START = 3;

Button::Button(ButtonType type, u32 x, u32 y, bool reuseTiles) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_buttonsTiles, sizeof(spr_buttonsTiles))
               .withSize(SIZE_32_32)
               .withLocation(x, y)
               .buildPtr();

  this->type = type;

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());

  SPRITE_goToFrame(sprite.get(), type);
}

void Button::setSelected(bool isSelected) {
  SPRITE_goToFrame(sprite.get(), (isSelected ? SELECTED_START : 0) + type);
}
