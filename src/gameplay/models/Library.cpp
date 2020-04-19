#include "Library.h"

#include "utils/StringUtils.h"

std::vector<std::unique_ptr<SongFile>> Library::getSongs() {
  const GBFS_FILE* fs = find_first_gbfs_file(0);
  u32 count = gbfs_count_objs(fs);
  std::vector<std::unique_ptr<SongFile>> files;

  for (u32 i = 0; i < count; i++) {
    char name[24];
    gbfs_get_nth_obj(fs, i, name, NULL);

    if (STRING_endsWith(name, METADATA_EXTENSION)) {
      auto fileName = STRING_removeFromEnd(std::string(name),
                                           sizeof(METADATA_EXTENSION) - 1);
      files.push_back(std::unique_ptr<SongFile>{new SongFile(fileName)});
    }
  }

  return files;
}
