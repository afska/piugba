#ifndef DEATH_MIX_H
#define DEATH_MIX_H

#include <array>
#include <memory>
#include <vector>

#include "Library.h"
#include "models/Song.h"
#include "objects/score/Score.h"

typedef struct {
  Song* song;
  Chart* chart;
} SongChart;

class DeathMix {
 public:
  bool didStartScroll = false;
  u32 multiplier = 1;
  u32 combo = 0;
  int life = INITIAL_LIFE;
  bool hasMissCombo = false;
  bool halfLifeBonus = false;
  u32 maxCombo = 0;
  std::array<u32, FEEDBACK_TYPES_TOTAL> counters;
  u32 points = 0;
  u32 longNotes = 0;

  DeathMix(const GBFS_FILE* fs, DifficultyLevel difficultyLevel);

  bool isInitialSong() { return next == 1; }
  SongChart getNextSongChart();
  u32 getCurrentSongNumber() { return next - 1; }

 private:
  const GBFS_FILE* fs;
  DifficultyLevel difficultyLevel;
  std::vector<std::unique_ptr<SongFile>> songFiles;
  u32 next;
  u32 total;
};

#endif  // DEATH_MIX_H
