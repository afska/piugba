#include "ComboTitle.h"

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_combo.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/ArrowInfo.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 16;
const u32 POSITION_Y = 70;

ComboTitle::ComboTitle(u8 playerId) {
  this->playerId = playerId;
  relocate();
  animationDirection = -1;

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_comboTiles, sizeof(spr_comboTiles))
               .withSize(SIZE_64_32)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();

  if (playerId > 0)
    SPRITE_reuseTiles(sprite.get());
}

void ComboTitle::relocate() {
  animationPositionX =
      (isDouble() ? GAME_POSITION_X[1] : GameState.positionX[playerId]) +
      POSITION_X;
  animationPositionY = GameState.scorePositionY + POSITION_Y;
}

Sprite* ComboTitle::get() {
  return sprite.get();
}
