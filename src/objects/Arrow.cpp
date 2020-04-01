#include "Arrow.h"
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/compiled/spr_arrows.h"
#include "utils/SpriteUtils.h"

const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ANIMATION_FRAMES = 5;
const u32 ANIMATION_DELAY = 2;
const u32 END_ANIMATION_START = 5;
const u32 END_ANIMATION_DELAY_MS = 30;

Arrow::Arrow(u32 id) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_arrowsTiles, sizeof(spr_arrowsTiles))
               .withSize(SIZE_16_16)
               .withAnimated(ANIMATION_FRAMES, ANIMATION_DELAY)
               .withLocation(HIDDEN_WIDTH, HIDDEN_HEIGHT)
               .buildPtr();
  sprite->enabled = false;

  if (id > 0)
    SPRITE_reuseTiles(sprite.get());
}

void Arrow::discard() {
  sprite->enabled = false;
}

void Arrow::initialize(ArrowType type) {
  u32 start = 0;
  bool flip = false;
  switch (type) {
    case ArrowType::DOWNLEFT:
      start = ARROW_FRAMES * 0;
      break;
    case ArrowType::UPLEFT:
      start = ARROW_FRAMES * 1;
      break;
    case ArrowType::CENTER:
      start = ARROW_FRAMES * 2;
      break;
    case ArrowType::UPRIGHT:
      start = ARROW_FRAMES * 1;
      flip = true;
      break;
    case ArrowType::DOWNRIGHT:
      start = ARROW_FRAMES * 0;
      flip = true;
      break;
  }

  this->type = type;
  this->start = start;
  this->flip = flip;

  sprite->moveTo(ARROW_CORNER_MARGIN_X + ARROW_MARGIN * type,
                 GBA_SCREEN_HEIGHT);
  sprite->makeAnimated(this->start, ANIMATION_FRAMES, ANIMATION_DELAY);
  sprite->enabled = true;
  msecs = 0;
  endTime = 0;
}

void Arrow::press() {
  endTime = msecs;
  sprite->moveTo(sprite->getX(), ARROW_CORNER_MARGIN_Y);
  SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START);
}

bool Arrow::isEnding() {
  return endTime > 0;
}

FeedbackType Arrow::tick(u32 msecs, bool isPressed) {
  this->msecs = msecs;
  sprite->flipHorizontally(flip);

  if (SPRITE_isHidden(sprite.get()))
    return FeedbackType::ENDED;

  if (isEnding()) {
    u32 diff = abs(msecs - endTime);

    if (diff > END_ANIMATION_DELAY_MS) {
      if (diff < END_ANIMATION_DELAY_MS * 2)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 1);
      else if (diff < END_ANIMATION_DELAY_MS * 3)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 2);
      else if (diff < END_ANIMATION_DELAY_MS * 4)
        SPRITE_goToFrame(sprite.get(), this->start + END_ANIMATION_START + 3);
      else {
        SPRITE_hide(sprite.get());
        sprite->stopAnimating();
      }
    }

    return FeedbackType::ENDING;
  } else if (sprite->getY() < ARROW_OFFSCREEN_LIMIT) {
    endTime = msecs;
    SPRITE_hide(sprite.get());
    sprite->stopAnimating();

    return FeedbackType::MISS;
  } else
    sprite->moveTo(sprite->getX(), sprite->getY() - ARROW_SPEED);

  return FeedbackType::ACTIVE;
}

Sprite* Arrow::get() {
  return sprite.get();
}
