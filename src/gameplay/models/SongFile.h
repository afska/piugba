#ifndef SONG_FILE_H
#define SONG_FILE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <string>

#define METADATA_EXTENSION ".pius"
#define BACKGROUND_TILES_EXTENSION ".img.bin"
#define BACKGROUND_PALETTE_EXTENSION ".pal.bin"
#define BACKGROUND_MAP_EXTENSION ".map.bin"
#define VIDEO_EXTENSION ".vid.bin"

typedef struct SongFile {
  u32 index;
  std::string name;

  SongFile(std::string name, u32 index) {
    this->index = index;
    this->name = name;
  }

  std::string getMetadataFile() { return name + METADATA_EXTENSION; }
  std::string getAudioFile() { return name; }  // (extensions: .gsm || .aud.bin)
  std::string getBackgroundTilesFile() {
    return name + BACKGROUND_TILES_EXTENSION;
  }
  std::string getBackgroundPaletteFile() {
    return name + BACKGROUND_PALETTE_EXTENSION;
  }
  std::string getBackgroundMapFile() { return name + BACKGROUND_MAP_EXTENSION; }
  std::string getVideoFile() { return name + VIDEO_EXTENSION; }
} SongFile;

#endif  // SONG_FILE_H
