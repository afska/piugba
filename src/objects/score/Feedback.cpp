#include "Feedback.h"

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_feedback.h"
#include "data/content/_compiled_sprites/spr_feedback_mdrn.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/ArrowInfo.h"
#include "utils/SpriteUtils.h"

const u32 POSITION_X = 16;
const u32 POSITION_Y = 60;

Feedback::Feedback(u8 playerId) {
  this->playerId = playerId;
  type = FeedbackType::MISS;
  relocate();

  SpriteBuilder<Sprite> builder;
  sprite = builder
               .withData(SAVEFILE_isUsingModernTheme() ? spr_feedback_mdrnTiles
                                                       : spr_feedbackTiles,
                         SAVEFILE_isUsingModernTheme()
                             ? sizeof(spr_feedback_mdrnTiles)
                             : sizeof(spr_feedbackTiles))
               .withSize(SIZE_64_32)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();

  if (playerId > 0)
    SPRITE_reuseTiles(sprite.get());
}

void Feedback::setType(FeedbackType type) {
  this->type = type;
  SPRITE_goToFrame(sprite.get(), type);
}

void Feedback::relocate() {
  animationPositionX =
      (isDouble() ? GAME_POSITION_X[1] : GameState.positionX[playerId]) +
      POSITION_X;
  animationPositionY = GameState.scorePositionY + POSITION_Y;
}

Sprite* Feedback::get() {
  return sprite.get();
}
