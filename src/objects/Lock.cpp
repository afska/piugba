#include "Lock.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_lock.h"
#include "utils/SpriteUtils.h"

Lock::Lock(u32 x, u32 y, bool reuseTiles) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_lockTiles, sizeof(spr_lockTiles))
               .withSize(SIZE_16_16)
               .withLocation(x, y)
               .buildPtr();

  this->x = x;
  this->y = y;

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());
}

bool Lock::isVisible() {
  return !SPRITE_isHidden(sprite.get());
}

void Lock::setVisible(bool isVisible) {
  if (isVisible)
    sprite->moveTo(x, y);
  else
    SPRITE_hide(sprite.get());
}
