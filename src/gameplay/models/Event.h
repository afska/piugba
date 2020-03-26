#ifndef EVENT_H
#define EVENT_H

#include <libgba-sprite-engine/gba/tonc_core.h>

typedef struct {
  u32 timestamp;  // in ms
  u8 data;
  /* {
        [bits 0-2] type (0 = NOTE,
                         1 = HOLD_START,
                         2 = HOLD_TAIL,
                         3 = STOP,
                         4 = SET_TEMPO,
                         5 = SET_SPEED,
                         ...rest = UNUSED
                         )
        [bits 3-7] data (5-bit array with the arrows)
      }
  */
} Event;

#endif  // EVENT_H
