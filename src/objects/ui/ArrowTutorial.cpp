#include "ArrowTutorial.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "data/content/_compiled_sprites/spr_arrows_mdrn.h"
#include "utils/SpriteUtils.h"

ArrowTutorial::ArrowTutorial(ArrowDirection direction) {
  u32 startTile = 0;
  u32 endTile = 0;
  ARROW_initialize(direction, startTile, endTile, this->flip);
  this->direction = direction;
  this->startTile = startTile;
  this->endTile = endTile;

  SpriteBuilder<Sprite> builder;
  sprite =
      builder
          .withData(SAVEFILE_isUsingModernTheme() ? spr_arrows_mdrnTiles
                                                  : spr_arrowsTiles,
                    SAVEFILE_isUsingModernTheme() ? sizeof(spr_arrows_mdrnTiles)
                                                  : sizeof(spr_arrowsTiles))
          .withSize(SIZE_16_16)
          .withLocation(0, 0)
          .buildPtr();

  SPRITE_reuseTiles(sprite.get());
  sprite->flipHorizontally(flip == ArrowFlip::FLIP_X ||
                           flip == ArrowFlip::FLIP_BOTH);
  sprite->flipVertically(flip == ArrowFlip::FLIP_Y ||
                         flip == ArrowFlip::FLIP_BOTH);
  SPRITE_goToFrame(sprite.get(), endTile);
}

void ArrowTutorial::tick() {
  SPRITE_goToFrame(sprite.get(), isOn ? startTile : endTile);
}
