#ifndef KEY_H
#define KEY_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include "multiplayer/PS2Keyboard.h"

inline bool KEY_DOWNLEFT(u16 keys) {
  return ((keys & KEY_DOWN) | (keys & KEY_LEFT)) || ps2Keyboard->arrows[0] ||
         ps2Keyboard->arrows[5];
}

inline bool KEY_UPLEFT(u16 keys) {
  return ((keys & KEY_L) | (keys & KEY_UP)) || ps2Keyboard->arrows[1] ||
         ps2Keyboard->arrows[6];
}

inline bool KEY_CENTER(u16 keys) {
  return ((keys & KEY_B) | (keys & KEY_RIGHT)) || ps2Keyboard->arrows[2] ||
         ps2Keyboard->arrows[7];
}

inline bool KEY_UPRIGHT(u16 keys) {
  return (keys & KEY_R) || ps2Keyboard->arrows[3] || ps2Keyboard->arrows[8];
}

inline bool KEY_DOWNRIGHT(u16 keys) {
  return (keys & KEY_A) || ps2Keyboard->arrows[4] || ps2Keyboard->arrows[9];
}

// TODO: IMPROVE INTEGRATION

inline bool KEY_ANY_PRESSED(u16 keys) {
  return KEY_DOWNLEFT(keys) || KEY_UPLEFT(keys) || KEY_CENTER(keys) ||
         KEY_UPRIGHT(keys) || KEY_DOWNRIGHT(keys);
}

#endif  // KEY_H
