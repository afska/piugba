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

enum Channel { ORIGINAL, KPOP, WORLD, BOSS };

typedef struct {
  char* title;          // 0x00 (31 bytes - including \0)
  char* artist;         // 0x1F (27 bytes - including \0)
  Channel channel;      // 0x3A (u8)
  u32 lastMillisecond;  // 0x3B (u32)
  u32 sampleStart;      // 0x3F (u32 - in ms)
  u32 sampleLength;     // 0x43 (u32 - in ms)

  u8 pixelate;        //   0x47
  u8 jump;            //   0x48
  u8 reduce;          //   0x49
  u8 negativeColors;  //   0x4A
  u8 randomSpeed;     //   0x4B
  u8 extraJudgement;  //   0x4C
  u8 hasMessage;      //   0x4D
  char* message;      //   0x4E (optional - 107 bytes - including \0)

  u8 chartCount;  // 0x4E if no message, 0xB9 otherwise (u8)
  Chart* charts;  // 0x4F if no message, 0xBA otherwise ("chartCount" times)

  // custom fields:
  u32 id;
  std::string audioPath;
  std::string backgroundTilesPath;
  std::string backgroundPalettePath;
  std::string backgroundMapPath;
} Song;

Song* SONG_parse(const GBFS_FILE* fs, SongFile* file, bool full);
Channel SONG_getChannel(const GBFS_FILE* fs, SongFile* file);
Chart* SONG_findChartByNumericLevelIndex(Song* song, u8 levelIndex);
Chart* SONG_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel);
void SONG_free(Song* song);

#endif  // SONG_H
