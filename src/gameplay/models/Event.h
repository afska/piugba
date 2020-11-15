#ifndef EVENT_H
#define EVENT_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#define EVENT_TYPE /*             */ 0b00000111
#define EVENT_ARROW_DOWNLEFT /*   */ 0b00001000
#define EVENT_ARROW_UPLEFT /*     */ 0b00010000
#define EVENT_ARROW_CENTER /*     */ 0b00100000
#define EVENT_ARROW_UPRIGHT /*    */ 0b01000000
#define EVENT_ARROW_DOWNRIGHT /*  */ 0b10000000

#define GAME_MAX_PLAYERS 2

enum EventType {
  SET_FAKE,
  NOTE,
  HOLD_START,
  HOLD_END,
  SET_TEMPO,
  SET_TICKCOUNT,
  STOP,
  WARP,
};
const u8 EVENT_ARROW_MASKS[] = {EVENT_ARROW_DOWNLEFT, EVENT_ARROW_UPLEFT,
                                EVENT_ARROW_CENTER, EVENT_ARROW_UPRIGHT,
                                EVENT_ARROW_DOWNRIGHT};

inline bool EVENT_HAS_PARAM(EventType event) {
  return event == EventType::SET_TEMPO || event == EventType::SET_TICKCOUNT ||
         event == EventType::SET_FAKE || event == EventType::STOP ||
         event == EventType::WARP;
}

inline bool EVENT_HAS_PARAM2(EventType event) {
  return event == EventType::SET_TEMPO || event == EventType::STOP;
}

inline bool EVENT_HAS_PARAM3(EventType event) {
  return event == EventType::SET_TEMPO;
}

typedef struct {
  int timestamp;  // in ms
  u8 data;
  /* {
        [bits 0-2] type (see EventType)
        [bits 3-7] data (5-bit array with the arrows)
      }
  */
  u32 param;
  u32 param2;
  u32 param3;
  // (params are not present in note-related events)
  u32 index = 0;
  bool handled[GAME_MAX_PLAYERS];
} Event;

#endif  // EVENT_H
