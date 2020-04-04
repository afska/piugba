#include "Song.h"

#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 40;
const u32 ARTIST_LEN = 15;

Song* Song_parse(const GBFS_FILE* fs, char* fileName) {
  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, fileName, &length);

  u32 cursor = 0;
  auto song = new Song();

  song->title = (char*)malloc(TITLE_LEN);
  parse_array(data, &cursor, song->title, TITLE_LEN);

  song->artist = (char*)malloc(ARTIST_LEN);
  parse_array(data, &cursor, song->artist, ARTIST_LEN);

  song->channel = static_cast<Channel>(parse_u8(data, &cursor));
  song->sampleStart = parse_u32le(data, &cursor);
  song->sampleLength = parse_u32le(data, &cursor);

  song->chartCount = parse_u8(data, &cursor);
  song->charts = (Chart*)malloc(sizeof(Chart) * song->chartCount);
  for (u32 i = 0; i < song->chartCount; i++) {
    auto chart = song->charts + i;

    chart->difficulty = static_cast<Difficulty>(parse_u8(data, &cursor));
    chart->level = parse_u8(data, &cursor);

    chart->eventCount = parse_u32le(data, &cursor);
    chart->events = (Event*)malloc(sizeof(Event) * chart->eventCount);

    for (u32 j = 0; j < chart->eventCount; j++) {
      auto event = chart->events + j;

      event->timestamp = parse_u32le(data, &cursor);
      event->data = parse_u8(data, &cursor);
    }
  }

  return song;
}

Chart* Song_findChartByLevel(Song* song, u8 level) {
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].level == level)
      return song->charts + i;
  }

  return NULL;
}

void Song_free(Song* song) {
  free(song->title);
  free(song->artist);

  for (u32 i = 0; i < song->chartCount; i++)
    free((song->charts + i)->events);
  free(song->charts);

  delete song;
}
