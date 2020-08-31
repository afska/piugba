#ifndef LIBRARY_H
#define LIBRARY_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include <memory>
#include <string>
#include <vector>

#include "models/Chart.h"
#include "models/SongFile.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

#define PREFIX_NORMAL "_snm_"
#define PREFIX_HARD "_shd_"
#define PREFIX_CRAZY "_scz_"
#define SUFFIX_LIST "_list.txt"

const u32 PAGE_SIZE = 4;

class Library {
 public:
  Library(const GBFS_FILE* fs);

  std::vector<std::unique_ptr<SongFile>> loadSongs(DifficultyLevel libraryType,
                                                   u32 pageStart);

  inline std::string getPrefix() {
    switch (libraryType) {
      case DifficultyLevel::NORMAL:
        return PREFIX_NORMAL;
      case DifficultyLevel::HARD:
        return PREFIX_HARD;
      default:
        return PREFIX_CRAZY;
    }
  }

 private:
  const GBFS_FILE* fs;
  DifficultyLevel libraryType;
};

#endif  // LIBRARY_H
