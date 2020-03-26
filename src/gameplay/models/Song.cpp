#include "Song.h"
#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 40;
const u32 ARTIST_LEN = 15;

void parse_array(u8* source, u32* cursor, void* target, u32 length) {
  memcpy(target, source + *cursor, length);
  *cursor += length;
}

void parse_u32(u8* source, u32* cursor, u32* target) {
  *target = *((u32*)(source + *cursor));
  *cursor += sizeof(u32);
}

Song* Song_parse(const GBFS_FILE* fs, char* fileName) {
  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, fileName, &length);

  u32 cursor = 0;
  auto song = new Song();

  song->title = (char*)malloc(TITLE_LEN);
  parse_array(data, &cursor, song->title, TITLE_LEN);

  song->artist = (char*)malloc(ARTIST_LEN);
  parse_array(data, &cursor, song->artist, ARTIST_LEN);

  // parse_array(data, &cursor, &song->channel, sizeof(u8));
  cursor += 1;

  parse_u32(data, &cursor, &song->sampleStart);
  parse_u32(data, &cursor, &song->sampleLength);

  return song;
}

void Song_free(Song* song) {
  free(song->title);
  free(song->artist);
  free(song);
}
