#ifndef SPRITE_UTILS_H
#define SPRITE_UTILS_H

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#define HIDDEN_WIDTH GBA_SCREEN_WIDTH
#define HIDDEN_HEIGHT GBA_SCREEN_HEIGHT

inline void SPRITE_enable() {
  REG_DISPCNT = REG_DISPCNT | DCNT_OBJ;
}

inline void SPRITE_disable() {
  REG_DISPCNT = REG_DISPCNT & ~DCNT_OBJ;
}

inline void SPRITE_hide(Sprite* sprite) {
  sprite->moveTo(HIDDEN_WIDTH, HIDDEN_HEIGHT);
}

inline bool SPRITE_isHidden(Sprite* sprite) {
  return sprite->getX() == HIDDEN_WIDTH && sprite->getY() == HIDDEN_HEIGHT;
}

inline void SPRITE_goToFrame(Sprite* sprite, int frame) {
  sprite->makeAnimated(0, 0, 0);
  sprite->stopAnimating();
  sprite->animateToFrame(frame);
}

inline void SPRITE_reuseTiles(Sprite* sprite) {
  sprite->setData(NULL);
  sprite->setImageSize(0);
}

#endif  // SPRITE_UTILS_H
