#ifndef LIBRARY_H
#define LIBRARY_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include <memory>
#include <string>
#include <vector>

#include "models/SongFile.h"
#include "utils/StringUtils.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class Library {
 public:
  Library(const GBFS_FILE* fs);

  std::vector<std::unique_ptr<SongFile>> getSongs(u32 start, u32 count);
  u32 getCount();

 private:
  const GBFS_FILE* fs;

  template <typename F>
  inline void forEachSong(F action) {
    forEachSong(0, 0, action);
  }

  template <typename F>
  inline void forEachSong(u32 start, u32 count, F action) {
    u32 fileCount = gbfs_count_objs(fs);

    u32 discoveredCount = 0;
    u32 processedCount = 0;
    for (u32 i = 0; i < fileCount; i++) {
      char fileName[24];
      bool exists = gbfs_get_nth_obj(fs, i, fileName, NULL);
      if (!exists)
        break;

      if (STRING_endsWith(fileName, METADATA_EXTENSION)) {
        if (count == 0 ||
            (discoveredCount >= start && processedCount < count)) {
          processedCount++;

          auto name = STRING_removeFromEnd(std::string(fileName),
                                           sizeof(METADATA_EXTENSION) - 1);
          action(name);
        }
        if (processedCount == count)
          break;

        discoveredCount++;
      }
    }
  }
};

#endif  // LIBRARY_H
