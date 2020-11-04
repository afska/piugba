#ifndef SONG_H
#define SONG_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Chart.h"
#include "Event.h"
#include "SongFile.h"
#include "gameplay/save/GameMode.h"
#include "utils/parse.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

enum Channel { ORIGINAL, KPOP, WORLD, BOSS };

typedef struct {
  u8 id;                // 0x00
  char* title;          // 0x01 (31 bytes - including \0)
  char* artist;         // 0x20 (27 bytes - including \0)
  Channel channel;      // 0x3B (u8)
  u32 lastMillisecond;  // 0x3C (u32)
  u32 sampleStart;      // 0x40 (u32 - in ms)
  u32 sampleLength;     // 0x44 (u32 - in ms)

  u8 applyTo[3];   //   0x48
  u8 isBoss;       //   0x4B
  u8 pixelate;     //   0x4C
  u8 jump;         //   0x4D
  u8 reduce;       //   0x4E
  u8 decolorize;   //   0x4F
  u8 randomSpeed;  //   0x50
  u8 ___;          //   0x51 (unused)
  u8 hasMessage;   //   0x52
  char* message;   //   0x53 (optional - 107 bytes - including \0)

  u8 chartCount;  // 0x53 if no message, 0xBE otherwise (u8)
  Chart* charts;  // 0x54 if no message, 0xBF otherwise ("chartCount" times)

  // custom fields:
  u32 index;
  std::string audioPath;
  std::string backgroundTilesPath;
  std::string backgroundPalettePath;
  std::string backgroundMapPath;
} Song;

Song* SONG_parse(const GBFS_FILE* fs, SongFile* file, bool full);
Channel SONG_getChannel(const GBFS_FILE* fs,
                        GameMode gameMode,
                        SongFile* file,
                        DifficultyLevel difficultyLevel);
Chart* SONG_findChartByNumericLevelIndex(Song* song, u8 levelIndex);
Chart* SONG_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel);
void SONG_free(Song* song);

#endif  // SONG_H
