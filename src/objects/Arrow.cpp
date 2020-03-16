#include "Arrow.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_arrow_center.h"
#include "data/content/compiled/spr_arrow_downleft.h"
#include "data/content/compiled/spr_arrow_upleft.h"
#include "utils/SpriteUtils.h";
#include <libgba-sprite-engine/gba/tonc_core.h> // TODO: REMOVE (qran_range)

const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 END_ANIMATION_START = 5;
const u32 END_ANIMATION_DELAY_MS = 40;

Arrow::Arrow(u32 id, ArrowType type) {
  const unsigned int* tiles;
  u32 size;
  bool flip = false;
  switch (type) {
    case ArrowType::DOWNLEFT:
      tiles = spr_arrow_downleftTiles;
      size = spr_arrow_downleftTilesLen;
      break;
    case ArrowType::UPLEFT:
      tiles = spr_arrow_upleftTiles;
      size = spr_arrow_upleftTilesLen;
      break;
    case ArrowType::CENTER:
      tiles = spr_arrow_centerTiles;
      size = spr_arrow_centerTilesLen;
      break;
    case ArrowType::UPRIGHT:
      tiles = spr_arrow_upleftTiles;
      size = spr_arrow_upleftTilesLen;
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      tiles = spr_arrow_downleftTiles;
      size = spr_arrow_downleftTilesLen;
      flip = true;
      break;
  }

  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(tiles, size)
               .withSize(SIZE_16_16)
               .withAnimated(ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();
  sprite->enabled = false;

  if (id > 0 || flip)
    SPRITE_reuseTiles(sprite.get());

  this->type = type;
  this->flip = flip;
}

void Arrow::initialize() {
  sprite->moveTo(ARROW_CORNER_MARGIN + ARROW_MARGIN * type, GBA_SCREEN_HEIGHT);
  sprite->makeAnimated(0, ANIMATION_FRAMES, ANIMATION_DELAY);
  sprite->enabled = true;
  endTime = 0;
  feedbackType = FeedbackType::ACTIVE;
}

void Arrow::discard() {
  sprite->enabled = false;
}

FeedbackType Arrow::tick(u32 millis, bool isPressed) {
  sprite->flipHorizontally(flip);

  if (SPRITE_isHidden(sprite.get()))
    return feedbackType;

  if (endTime > 0) {
    u32 diff = abs(millis - endTime);

    if (diff > END_ANIMATION_DELAY_MS) {
      if (diff < END_ANIMATION_DELAY_MS * 2)
        SPRITE_goToFrame(sprite.get(), END_ANIMATION_START + 1);
      else if (diff < END_ANIMATION_DELAY_MS * 3)
        SPRITE_goToFrame(sprite.get(), END_ANIMATION_START + 2);
      else if (diff < END_ANIMATION_DELAY_MS * 4)
        SPRITE_goToFrame(sprite.get(), END_ANIMATION_START + 3);
      else {
        SPRITE_hide(sprite.get());
        sprite->stopAnimating();
      }
    }
  } else if (abs(sprite->getY() - ARROW_CORNER_MARGIN) < 3) {
    endTime = millis;
    feedbackType = static_cast<FeedbackType>(qran_range(0, 5)); // TODO: Use isPressed or remove
    SPRITE_goToFrame(sprite.get(), END_ANIMATION_START);
  } else
    sprite->moveTo(sprite->getX(), sprite->getY() - 3);

  return FeedbackType::ACTIVE;
}

Sprite* Arrow::get() {
  return sprite.get();
}
