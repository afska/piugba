#include "NumericLevelDeathMix.h"

NumericLevelDeathMix::NumericLevelDeathMix(const GBFS_FILE* fs, u8 numericLevel)
    : DeathMix(fs, MixMode::SHUFFLE) {
  this->numericLevel = numericLevel;
}

int NumericLevelDeathMix::getNextChartIndex(Song* tempSong) {
  return SONG_findSingleChartIndexByNumericLevel(tempSong, numericLevel);
}
