#include "AnimatedIndicator.h"
#include "utils/SpriteUtils.h"

const u32 ANIMATION_FRAMES = 4;
const u32 TOTAL_DURATION_MS = 750;
const u32 TOTAL_FRAMES = TOTAL_DURATION_MS * 60 / 1000;

void AnimatedIndicator::show() {
  currentFrame = 0;
  animationFrame = 0;
  get()->moveTo(animationPositionX,
                animationPositionY - animationDirection * ANIMATION_FRAMES);
}

void AnimatedIndicator::tick() {
  if (SpriteUtils::isHidden(get()))
    return;

  currentFrame++;

  if (currentFrame < ANIMATION_FRAMES) {
    // in animation
    animationFrame++;
    get()->moveTo(animationPositionX,
                  animationPositionY -
                      animationDirection * (ANIMATION_FRAMES - animationFrame));

    if (animationFrame == ANIMATION_FRAMES)
      animationFrame = 0;
  } else if (currentFrame > TOTAL_FRAMES) {
    // out animation
    animationFrame++;
    get()->moveTo(animationPositionX,
                  animationPositionY - animationDirection * animationFrame);

    if (animationFrame == ANIMATION_FRAMES)
      SpriteUtils::hide(get());
  }
}
