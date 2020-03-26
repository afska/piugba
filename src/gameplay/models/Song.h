#ifndef SONG_H
#define SONG_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include "Chart.h"
#include "Event.h"

extern "C" {
#include "utils/gbfs.h"
}

enum Channel { ORIGINAL, KPOP, WORLD };

typedef struct {
  char* title;       // (40 bytes)
  char* artist;      // (15 bytes)
  Channel channel;   // (u8)
  u32 sampleStart;   // in ms
  u32 sampleLength;  // in ms

  u8 length;
  Chart* charts;  // ("length" times)
} Song;

Song* Song_parse(const GBFS_FILE* fs, char* fileName);
void Song_free();

#endif  // SONG_H
