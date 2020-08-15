#include "Song.h"

#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 31;
const u32 ARTIST_LEN = 27;

Song* SONG_parse(const GBFS_FILE* fs, SongFile* file, bool full) {
  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, file->getMetadataFile().c_str(), &length);

  u32 cursor = 0;
  auto song = new Song();

  song->title = (char*)malloc(TITLE_LEN);
  parse_array(data, &cursor, song->title, TITLE_LEN);

  song->artist = (char*)malloc(ARTIST_LEN);
  parse_array(data, &cursor, song->artist, ARTIST_LEN);

  song->channel = static_cast<Channel>(parse_u8(data, &cursor));
  song->lastMillisecond = parse_u32le(data, &cursor);
  song->sampleStart = parse_u32le(data, &cursor);
  song->sampleLength = parse_u32le(data, &cursor);

  song->chartCount = parse_u8(data, &cursor);
  song->charts = (Chart*)malloc(sizeof(Chart) * song->chartCount);
  for (u32 i = 0; i < song->chartCount; i++) {
    auto chart = song->charts + i;

    chart->difficulty = static_cast<DifficultyLevel>(parse_u8(data, &cursor));
    chart->level = parse_u8(data, &cursor);

    chart->eventChunkSize = parse_u32le(data, &cursor);
    if (!full) {
      chart->eventCount = 0;
      cursor += chart->eventChunkSize;
      continue;
    }

    chart->eventCount = parse_u32le(data, &cursor);
    chart->events = (Event*)malloc(sizeof(Event) * chart->eventCount);

    for (u32 j = 0; j < chart->eventCount; j++) {
      auto event = chart->events + j;

      event->timestamp = parse_s32le(data, &cursor);
      event->data = parse_u8(data, &cursor);
      auto eventType = static_cast<EventType>(event->data & EVENT_TYPE);
      if (EVENT_HAS_PARAM(eventType))
        event->param = parse_u32le(data, &cursor);
      if (EVENT_HAS_PARAM2(eventType))
        event->param2 = parse_u32le(data, &cursor);
      if (EVENT_HAS_PARAM3(eventType))
        event->param3 = parse_u32le(data, &cursor);
      event->handled = false;
    }
  }

  song->id = file->id;
  song->audioPath = file->getAudioFile();
  song->backgroundTilesPath = file->getBackgroundTilesFile();
  song->backgroundPalettePath = file->getBackgroundPaletteFile();
  song->backgroundMapPath = file->getBackgroundMapFile();

  return song;
}

Channel SONG_getChannel(const GBFS_FILE* fs, SongFile* file) {
  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, file->getMetadataFile().c_str(), &length);

  u32 cursor = TITLE_LEN + ARTIST_LEN;
  return static_cast<Channel>(parse_u8(data, &cursor));
}

Chart* SONG_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel) {
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].difficulty == difficultyLevel)
      return song->charts + i;
  }

  return NULL;
}

Chart* SONG_findChartByNumericLevel(Song* song, u8 level) {
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].level == level)
      return song->charts + i;
  }

  return NULL;
}

void SONG_free(Song* song) {
  free(song->title);
  free(song->artist);

  for (u32 i = 0; i < song->chartCount; i++) {
    if ((song->charts + i)->eventCount > 0)
      free((song->charts + i)->events);
  }
  free(song->charts);

  delete song;
}
