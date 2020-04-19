#ifndef LIBRARY_H
#define LIBRARY_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <memory>
#include <string>
#include <vector>

#include "SongFile.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class Library {
 public:
  std::vector<std::unique_ptr<SongFile>> getSongs();
};

#endif  // LIBRARY_H
