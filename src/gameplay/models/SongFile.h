#ifndef SONG_FILE_H
#define SONG_FILE_H

#include <string>

typedef struct SongFile {
  std::string name;

  SongFile(std::string name) { this->name = name; }

  std::string getMetadataFile() { return name + ".pius"; }
  std::string getAudioFile() { return name + ".gsm"; }
  std::string getBackgroundTilesFile() { return name + ".img.bin"; }
  std::string getBackgroundPaletteFile() { return name + ".pal.bin"; }
  std::string getBackgroundMapFile() { return name + ".map.bin"; }
} SongFile;

#endif  // SONG_FILE_H
