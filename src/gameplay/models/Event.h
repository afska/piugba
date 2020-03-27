#ifndef EVENT_H
#define EVENT_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum EventType { NOTE, HOLD_START, HOLD_TAIL, STOP, SET_TEMPO, SET_SPEED };

#define EVENT_ARROW_TYPE /*       */ 0b00000111
#define EVENT_ARROW_DOWNLEFT /*   */ 0b00001000
#define EVENT_ARROW_UPLEFT /*     */ 0b00010000
#define EVENT_ARROW_CENTER /*     */ 0b00100000
#define EVENT_ARROW_UPRIGHT /*    */ 0b01000000
#define EVENT_ARROW_DOWNRIGHT /*  */ 0b10000000

typedef struct {
  u32 timestamp;  // in ms
  u8 data;
  /* {
        [bits 0-2] type (see EnumType)
        [bits 3-7] data (5-bit array with the arrows)
      }
  */
} Event;

#endif  // EVENT_H
