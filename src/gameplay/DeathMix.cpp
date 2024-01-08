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

  for (u32 i = 0; i < songFiles.size(); i++)
    songFiles[i]->index = i;

  this->fs = fs;
  this->difficultyLevel = difficultyLevel;
  this->current = 0;
  this->total = librarySize;
}

SongChart DeathMix::getNextSongChart() {
  if (current >= total)
    return SongChart{.song = NULL, .chart = NULL};

  Song* tempSong = SONG_parse(fs, songFiles[current].get());
  u8 index = SONG_findChartIndexByDifficultyLevel(tempSong, difficultyLevel);
  SONG_free(tempSong);
  Song* song = SONG_parse(fs, songFiles[current].get(), std::vector<u8>{index});
  Chart* chart = song->charts + index;

  current++;

  return SongChart{.song = song, .chart = chart};
}
