#include "Explosion.h"
#include "objects/Arrow.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "data/content/_compiled_sprites/spr_arrows_mdrn.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_TOTAL_FRAMES = 18;
const u8 ANIMATION_FRAMES[] = {
    EXPLOSION_ANIMATION_START + 0, EXPLOSION_ANIMATION_START + 0,
    EXPLOSION_ANIMATION_START + 0, EXPLOSION_ANIMATION_START + 1,
    EXPLOSION_ANIMATION_START + 1, EXPLOSION_ANIMATION_START + 1,
    EXPLOSION_ANIMATION_START + 2, EXPLOSION_ANIMATION_START + 2,
    EXPLOSION_ANIMATION_START + 2, EXPLOSION_ANIMATION_START + 3,
    EXPLOSION_ANIMATION_START + 3, EXPLOSION_ANIMATION_START + 3,
    EXPLOSION_ANIMATION_START + 2, EXPLOSION_ANIMATION_START + 2,
    EXPLOSION_ANIMATION_START + 2, EXPLOSION_ANIMATION_START + 1,
    EXPLOSION_ANIMATION_START + 1, EXPLOSION_ANIMATION_START + 1};

Explosion::Explosion(u32 x, u32 y, bool reuseTiles) {
  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(SAVEFILE_isUsingModernTheme() ? spr_arrows_mdrnTiles
                                                  : spr_arrowsTiles,
                    SAVEFILE_isUsingModernTheme() ? sizeof(spr_arrows_mdrnTiles)
                                                  : sizeof(spr_arrowsTiles))
          .withAnimated(15, 4, 2)
          .withSize(SIZE_16_16)
          .withLocation(x, y)
          .buildPtr();

  this->x = x;
  this->y = y;
  this->animationFrame = 0;

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());
}

bool Explosion::isVisible() {
  return !SPRITE_isHidden(sprite.get());
}

void Explosion::setVisible(bool isVisible) {
  if (isVisible) {
    sprite->moveTo(x, y);
    animationFrame = 0;
  } else
    SPRITE_hide(sprite.get());
}

void Explosion::tick() {
  SPRITE_goToFrame(sprite.get(), ANIMATION_FRAMES[animationFrame]);

  animationFrame++;
  if (animationFrame == ANIMATION_TOTAL_FRAMES)
    animationFrame = 0;
}
