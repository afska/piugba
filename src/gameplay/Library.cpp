#include "Library.h"

#include "utils/StringUtils.h"

Library::Library(const GBFS_FILE* fs) {
  this->fs = fs;
}

void Library::loadSongs(std::vector<std::unique_ptr<SongFile>>& files,
                        DifficultyLevel libraryType,
                        u32 pageStart) {
  this->libraryType = libraryType;

  std::string listFileName =
      getPrefix() + std::to_string(pageStart) + SUFFIX_LIST;
  auto library =
      std::string((char*)gbfs_get_obj(fs, listFileName.c_str(), NULL));
  auto fileNames = STRING_split(library, LINE_BREAK);

  for (u32 i = 0; i < fileNames.size(); i++)
    files.push_back(
        std::unique_ptr<SongFile>{new SongFile(fileNames[i], pageStart + i)});
}
