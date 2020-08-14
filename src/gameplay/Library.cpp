#include "Library.h"

Library::Library(const GBFS_FILE* fs) {
  this->fs = fs;
}

std::vector<std::unique_ptr<SongFile>> Library::getSongs(u32 start, u32 count) {
  std::vector<std::unique_ptr<SongFile>> files;

  forEachSong(start, count, [&files](std::string name, u32 index) {
    files.push_back(std::unique_ptr<SongFile>{new SongFile(name, index)});
  });

  return files;
}

u32 Library::getCount() {
  u32 count = 0;
  forEachSong([&count](std::string name, u32 index) { count++; });
  return count;
}
