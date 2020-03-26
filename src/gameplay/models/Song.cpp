#include "Song.h"
#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 40;
const u32 ARTIST_LEN = 15;

void parse_array(u8* source, u32* cursor, void* target, u32 length) {
  memcpy(target, source + *cursor, length);
  *cursor += length;
}

void parse_u8(u8* source, u32* cursor, u8* target) {
  *target = *(source + *cursor);
  *cursor += sizeof(u8);
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

  parse_u8(data, &cursor, (u8*)&song->channel);
  parse_u32(data, &cursor, &song->sampleStart);
  parse_u32(data, &cursor, &song->sampleLength);

  parse_u8(data, &cursor, &song->length);
  song->charts = (Chart*)malloc(sizeof(Chart) * song->length);
  for (u32 i = 0; i < song->length; i++) {
    auto chart = song->charts + i;

    parse_u8(data, &cursor, (u8*)&chart->difficulty);
    parse_u8(data, &cursor, &chart->level);

    parse_u32(data, &cursor, &chart->length);
    chart->events = (Event*)malloc(sizeof(Event) * chart->length);
    for (u32 j = 0; j < chart->length; j++) {
      auto event = chart->events + j;

      parse_u32(data, &cursor, &event->timestamp);
      parse_u8(data, &cursor, &event->data);
    }
  }

  return song;
}

void Song_free(Song* song) {
  free(song->title);
  free(song->artist);

  for (u32 i = 0; i < song->length; i++) {
    for (u32 j = 0; j < song->charts[i].length; j++)
      free((song->charts + i)->events);
  }
  free(song->charts);

  free(song);
}
