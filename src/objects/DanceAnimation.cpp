#include "DanceAnimation.h"
#include <libgba-sprite-engine/background/text_stream.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/arrow_center.h"

const u32 STEPS = 4;

DanceAnimation::DanceAnimation(u32 x, u32 y) {
  SpriteBuilder<Sprite> builder;

  sprite = builder.withData(arrow_centerTiles, sizeof(arrow_centerTiles))
               .withSize(SIZE_16_16)
               .withAnimated(5, 2)
               .withLocation(x, y)
               .buildPtr();
}

void DanceAnimation::update(u32 beat) {
  int delta = sgn(velocity);
  count += delta;
  sprite->moveTo(sprite->getX() + velocity, sprite->getY());

  // TODO: REMOVE
  int is_odd = beat & 1;
  TextStream::instance().setText("----------", !is_odd ? 19 : 18, 1);
  TextStream::instance().setText("oooooooooo", is_odd ? 19 : 18, 1);
  TextStream::instance().setText(std::to_string(beat), 9, 15);

  if (abs(count) >= STEPS) {
    velocity = -velocity;
    toLeft = !toLeft;
    sprite->flipHorizontally(toLeft);
  }
}

Sprite* DanceAnimation::get() {
  return sprite.get();
}
