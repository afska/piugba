#ifndef SONG_FILE_H
#define SONG_FILE_H

#include <string>

#define METADATA_EXTENSION ".pius"
#define AUDIO_EXTENSION ".gsm"
#define BACKGROUND_TILES_EXTENSION ".img.bin"
#define BACKGROUND_PALETTE_EXTENSION ".pal.bin"
#define BACKGROUND_MAP_EXTENSION ".map.bin"

typedef struct SongFile {
  std::string name;

  SongFile(std::string name) { this->name = name; }

  std::string getMetadataFile() { return name + METADATA_EXTENSION; }
  std::string getAudioFile() { return name + AUDIO_EXTENSION; }
  std::string getBackgroundTilesFile() {
    return name + BACKGROUND_TILES_EXTENSION;
  }
  std::string getBackgroundPaletteFile() {
    return name + BACKGROUND_PALETTE_EXTENSION;
  }
  std::string getBackgroundMapFile() { return name + BACKGROUND_MAP_EXTENSION; }
} SongFile;

#endif  // SONG_FILE_H
