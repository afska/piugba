#ifndef IOPORT_H
#define IOPORT_H

#include <libgba-sprite-engine/gba/tonc_core.h>

inline void IOPORT_sdHigh() {
  REG_RCNT |= 0b10;
}

inline void IOPORT_sdLow() {
  REG_RCNT &= 0b1111111111111101;
}

#endif  // IOPORT_H
