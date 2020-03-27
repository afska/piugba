#include "Song.h"
#include <libgba-sprite-engine/background/text_stream.h>  // TODO: log_text
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

void parse_u32le(u8* source, u32* cursor, u32* target) {
  u8* data = source + *cursor;
  *target =
      *data + (*(data + 1) << 8) + (*(data + 2) << 16) + (*(data + 2) << 24);
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
  parse_u32le(data, &cursor, &song->sampleStart);
  parse_u32le(data, &cursor, &song->sampleLength);

  parse_u8(data, &cursor, &song->length);
  song->charts = (Chart*)malloc(sizeof(Chart) * song->length);
  for (u32 i = 0; i < song->length; i++) {
    auto chart = song->charts + i;

    parse_u8(data, &cursor, (u8*)&chart->difficulty);
    parse_u8(data, &cursor, &chart->level);

    parse_u32le(data, &cursor, &chart->length);
    chart->events = (Event*)malloc(sizeof(Event) * chart->length);

    for (u32 j = 0; j < chart->length; j++) {
      auto event = chart->events + j;

      if (chart->level == 7 && j == 200) {
        u32 value = *(data + cursor) + (*(data + cursor + 1) << 8) +
                    (*(data + cursor + 2) << 16) + (*(data + cursor + 3) << 24);
        // TODO: Acá lo leo bien! Mantener la data en ROM y no RAM
        log_text(std::to_string(value).c_str());
      }

      parse_u32le(data, &cursor, &event->timestamp);
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
