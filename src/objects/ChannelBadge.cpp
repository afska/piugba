#include "ChannelBadge.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_channels.h"
#include "utils/SpriteUtils.h"

ChannelBadge::ChannelBadge(u32 x, u32 y, bool reuseTiles) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_channelsTiles, sizeof(spr_channelsTiles))
               .withSize(SIZE_16_16)
               .withLocation(x, y)
               .buildPtr();

  this->x = x;
  this->y = y;

  SPRITE_goToFrame(sprite.get(), 0);

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());
}

void ChannelBadge::setType(Channel type) {
  SPRITE_goToFrame(sprite.get(), type);
  sprite->moveTo(x, y);
}

void ChannelBadge::hide() {
  SPRITE_hide(sprite.get());
}
