#include "ArrowHolder.h"

#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_arrows.h"
#include "data/content/_compiled_sprites/spr_arrows_mdrn.h"
#include "utils/SpriteUtils.h"

ArrowHolder::ArrowHolder(ArrowDirection direction,
                         u8 playerId,
                         bool reuseTiles) {
  u32 startTile = 0;
  u32 endTile = 0;
  ARROW_initialize(direction, startTile, endTile, this->flip);
  this->direction = direction;
  this->playerId = playerId;
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
          .withLocation(
              ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction,
              ARROW_FINAL_Y())
          .buildPtr();

  if (reuseTiles)
    SPRITE_reuseTiles(sprite.get());

  SPRITE_goToFrame(sprite.get(), endTile);
  ARROW_setUpOrientation(sprite.get(), flip);
}

void ArrowHolder::blink() {
  isBlinking = true;
}

void ArrowHolder::tick(int offsetX) {
  u32 currentFrame = sprite->getCurrentFrame();
  u32 idleFrame = endTile;
  u32 pressedFrame = endTile + ARROW_HOLDER_PRESSED_OFFSET;

  sprite->moveTo(
      ARROW_CORNER_MARGIN_X(playerId) + ARROW_MARGIN * direction + offsetX,
      ARROW_FINAL_Y());

  if ((isPressed || isBlinking) && currentFrame < pressedFrame) {
    SPRITE_goToFrame(sprite.get(), currentFrame + 1);

    if (currentFrame + 1 == pressedFrame)
      isBlinking = false;
  } else if (!isPressed && currentFrame > idleFrame)
    SPRITE_goToFrame(sprite.get(), currentFrame - 1);
}
