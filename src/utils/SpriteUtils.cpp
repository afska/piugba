#include "SpriteUtils.h"

void SpriteUtils::hide(Sprite* sprite) {
  sprite->moveTo(HIDDEN_WIDTH, HIDDEN_HEIGHT);
}

bool SpriteUtils::isHidden(Sprite* sprite) {
  return sprite->getX() == HIDDEN_WIDTH && sprite->getY() == HIDDEN_HEIGHT;
}

void SpriteUtils::goToFrame(Sprite* sprite, int frame) {
  sprite->makeAnimated(0, 0, 0);
  sprite->stopAnimating();
  sprite->animateToFrame(frame);
}

void SpriteUtils::reuseTiles(Sprite* sprite) {
  sprite->setData(NULL);
  sprite->setImageSize(0);
}
