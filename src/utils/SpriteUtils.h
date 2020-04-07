#ifndef SPRITE_UTILS_H
#define SPRITE_UTILS_H

#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#define HIDDEN_WIDTH GBA_SCREEN_WIDTH - 1
#define HIDDEN_HEIGHT GBA_SCREEN_HEIGHT - 1

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

inline void BACKGROUND1_DISABLE() {
  REG_DISPCNT &= ~DCNT_BG1;
}

inline void BACKGROUND2_DISABLE() {
  REG_DISPCNT &= ~DCNT_BG2;
}

inline void BACKGROUND3_DISABLE() {
  REG_DISPCNT &= ~DCNT_BG3;
}

inline void BACKGROUND1_ENABLE() {
  REG_DISPCNT |= DCNT_BG1;
}

inline void BACKGROUND2_ENABLE() {
  REG_DISPCNT |= DCNT_BG2;
}

inline void BACKGROUND3_ENABLE() {
  REG_DISPCNT |= DCNT_BG3;
}

#endif  // SPRITE_UTILS_H
