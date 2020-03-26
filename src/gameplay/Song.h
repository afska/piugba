#ifndef SONG_H
#define SONG_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum Channel { ORIGINAL, KPOP, WORLD };
enum Difficulty { NORMAL, HARD, CRAZY, NUMERIC };

typedef struct {
  char* name;        // (40 bytes)
  char* artist;      // (15 bytes)
  Channel channel;   // (0 = ORIGINAL, 1 = K-POP, 2 = WORLD)
  u32 sampleStart;   // in ms
  u32 sampleLength;  // in ms

  u8 length;
  Chart* chart;  // ("length" times)
} Song;

typedef struct {
  u8 difficulty;  // (0 = NORMAL, 1 = HARD, 2 = CRAZY, 3 = NUMERIC)
  u8 level;       // 0-30

  u32 length;
  Event* events;  // ("length" times)
} Chart;

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

Song* song_parse(char* fileName) {}

void song_free() {}

#endif  // SONG_H
