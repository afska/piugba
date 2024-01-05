#include "DeathMix.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include "save/SaveFile.h"

DeathMix::DeathMix(const GBFS_FILE* fs, DifficultyLevel difficultyLevel) {
  auto library = std::unique_ptr<Library>{new Library(fs)};
  u32 librarySize = SAVEFILE_getLibrarySize();
  u32 pages =
      Div(librarySize, PAGE_SIZE) + (DivMod(librarySize, PAGE_SIZE) > 0);

  for (u32 i = 0; i < pages; i++)
    library->loadSongs(songFiles, difficultyLevel, i * PAGE_SIZE);

  for (int i = songFiles.size() - 1; i > 0; --i) {
    int j = qran_range(0, i + 1);
    std::swap(songFiles[i], songFiles[j]);
  }

  this->fs = fs;
  this->difficultyLevel = difficultyLevel;
  this->current = 0;
  this->total = librarySize;
}

SongChart DeathMix::getNextSongChart() {
  if (current == total - 1)
    return SongChart{.song = NULL, .chart = NULL};

  Song* song =
      SONG_parse(fs, songFiles[current].get(), true, std::vector<u8>{});
  Chart* chart = SONG_findChartByDifficultyLevel(song, difficultyLevel);

  current++;

  return SongChart{.song = song, .chart = chart};
}
