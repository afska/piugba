#include "Song.h"
#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 40;
const u32 ARTIST_LEN = 15;

Song* Song_parse(const GBFS_FILE* fs, char* fileName) {
  u32 length;
  auto data = (char*)gbfs_get_obj(fs, fileName, &length);

  u32 read = 0;
  auto song = new Song();

  song->name = (char*)malloc(TITLE_LEN);
  memcpy(song->name, data, TITLE_LEN);
  read += TITLE_LEN;

  song->artist = (char*)malloc(ARTIST_LEN);
  memcpy(song->artist, data + read, ARTIST_LEN);

  return song;
}

void Song_free(Song* song) {}
