#ifndef DEATH_MIX_H
#define DEATH_MIX_H

#include "Library.h"
#include "gameplay/models/Song.h"

#include <vector>

typedef struct {
  Song* song;
  Chart* chart;
} SongChart;

class DeathMix {
 public:
  DeathMix(const GBFS_FILE* fs, DifficultyLevel difficultyLevel);

  SongChart getNextSongChart();

 private:
  const GBFS_FILE* fs;
  DifficultyLevel difficultyLevel;
  std::vector<std::unique_ptr<SongFile>> songFiles;
  u32 current;
  u32 total;
};

#endif  // DEATH_MIX_H
