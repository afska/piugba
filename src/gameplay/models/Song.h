#ifndef SONG_H
#define SONG_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Chart.h"
#include "Event.h"
#include "SongFile.h"
#include "utils/parse.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

enum Channel { ORIGINAL, KPOP, WORLD, OTHER };

typedef struct {
  char* title;      // (31 bytes)
  char* artist;     // (27 bytes)
  Channel channel;  // (u8)
  u32 lastMillisecond;
  u32 sampleStart;   // in ms
  u32 sampleLength;  // in ms

  u8 chartCount;
  Chart* charts;  // ("chartCount" times)

  // custom fields:
  std::string audioPath;
  std::string backgroundTilesPath;
  std::string backgroundPalettePath;
  std::string backgroundMapPath;
} Song;

Song* Song_parse(const GBFS_FILE* fs, SongFile* file);
Chart* Song_findChartByNumericLevel(Song* song, u8 level);
Chart* Song_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel);
void Song_free(Song* song);

#endif  // SONG_H
