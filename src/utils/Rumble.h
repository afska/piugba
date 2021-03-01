#ifndef RUMBLE_H
#define RUMBLE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#define IO_DIRECTION ((u8*)0x80000C6)
#define IO_DATA ((u8*)0x80000C4)

inline void RUMBLE_start() {
  *IO_DATA = (*IO_DATA) | 0b1000;  // I/O bit 3 => HIGH
}

inline void RUMBLE_stop() {
  *IO_DATA = (*IO_DATA) & 0b11110111;  // I/O bit 3 => LOW
}

inline void RUMBLE_init() {
  *IO_DIRECTION = (*IO_DIRECTION) | 0b1000;  // I/O bit 3 => OUTPUT
  RUMBLE_stop();
}

#endif  // RUMBLE_H
