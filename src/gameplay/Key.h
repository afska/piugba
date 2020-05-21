#ifndef KEY_H
#define KEY_H

#include <libgba-sprite-engine/gba/tonc_core.h>

inline bool KEY_DOWNLEFT(u16 keys) {
  return (keys & KEY_DOWN) | (keys & KEY_LEFT);
}

inline bool KEY_UPLEFT(u16 keys) {
  return (keys & KEY_L) | (keys & KEY_UP);
}

inline bool KEY_CENTER(u16 keys) {
  return (keys & KEY_B) | (keys & KEY_RIGHT);
}

inline bool KEY_UPRIGHT(u16 keys) {
  return keys & KEY_R;
}

inline bool KEY_DOWNRIGHT(u16 keys) {
  return keys & KEY_A;
}

inline bool KEY_ANY_PRESSED(u16 keys) {
  return KEY_DOWNLEFT(keys) || KEY_UPLEFT(keys) || KEY_CENTER(keys) ||
         KEY_UPRIGHT(keys) || KEY_DOWNRIGHT(keys);
}

#endif  // KEY_H
