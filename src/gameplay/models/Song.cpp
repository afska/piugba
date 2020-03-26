#include "Song.h"
#include <string.h>

Song* Song_parse(const GBFS_FILE* fs, char* fileName) {
  u32 length;
  auto data = (char*)gbfs_get_obj(fs, fileName, &length);

  auto song = new Song();
  memcpy(song->name, data, 40);
  memcpy(song->artist, data + 40, 15);

  return song;
}

void Song_free(Song* song) {}
