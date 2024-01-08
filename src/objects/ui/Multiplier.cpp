#include "Multiplier.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_multipliers.h"
#include "objects/ArrowInfo.h"
#include "utils/SpriteUtils.h"

const u32 BLINK_FRAME = 6;

Multiplier::Multiplier(u32 x, u32 y, u32 initialValue) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_multipliersTiles, sizeof(spr_multipliersTiles))
               .withSize(SIZE_16_16)
               .withLocation(x, y)
               .buildPtr();

  value = initialValue;
  SPRITE_goToFrame(sprite.get(), getFrame());
}

u32 Multiplier::change() {
  value = DivMod(((value - 1) + 1), ARROW_MAX_MULTIPLIER) + 1;
  SPRITE_goToFrame(sprite.get(), BLINK_FRAME);
  return value;
}

void Multiplier::tick() {
  if (sprite->getCurrentFrame() == BLINK_FRAME)
    SPRITE_goToFrame(sprite.get(), getFrame());
}
