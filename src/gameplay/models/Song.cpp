#include "Song.h"

#include <stdlib.h>
#include <string.h>

const u32 TITLE_LEN = 31;
const u32 ARTIST_LEN = 27;
const u32 MESSAGE_LEN = 107;

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

  song->applyTo[DifficultyLevel::NORMAL] = parse_u8(data, &cursor);
  song->applyTo[DifficultyLevel::HARD] = parse_u8(data, &cursor);
  song->applyTo[DifficultyLevel::CRAZY] = parse_u8(data, &cursor);
  song->isBoss = parse_u8(data, &cursor);
  song->pixelate = parse_u8(data, &cursor);
  song->jump = parse_u8(data, &cursor);
  song->reduce = parse_u8(data, &cursor);
  song->negativeColors = parse_u8(data, &cursor);
  song->randomSpeed = parse_u8(data, &cursor);
  cursor += sizeof(u8);
  song->hasMessage = parse_u8(data, &cursor);

  if (song->hasMessage) {
    song->message = (char*)malloc(MESSAGE_LEN);
    parse_array(data, &cursor, song->message, MESSAGE_LEN);
  }

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

Channel SONG_getChannel(const GBFS_FILE* fs,
                        GameMode gameMode,
                        SongFile* file,
                        DifficultyLevel difficultyLevel) {
  if (gameMode == GameMode::IMPOSSIBLE)
    return Channel::BOSS;

  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, file->getMetadataFile().c_str(), &length);

  u32 cursor = TITLE_LEN + ARTIST_LEN;
  auto channel = static_cast<Channel>(parse_u8(data, &cursor));

  if (gameMode == GameMode::ARCADE)
    return channel;

  cursor +=
      4 /* lastMillisecond */ + 4 /* sampleStart */ + 4 /* sampleLength */;

  bool applyToNormal = parse_u8(data, &cursor);
  if (difficultyLevel == DifficultyLevel::NORMAL && !applyToNormal)
    return channel;

  bool applyToHard = parse_u8(data, &cursor);
  if (difficultyLevel == DifficultyLevel::HARD && !applyToHard)
    return channel;

  bool applyToCrazy = parse_u8(data, &cursor);
  if (difficultyLevel == DifficultyLevel::CRAZY && !applyToCrazy)
    return channel;

  bool isBoss = parse_u8(data, &cursor);
  return isBoss ? Channel::BOSS : channel;
}

Chart* SONG_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel) {
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].difficulty == difficultyLevel)
      return song->charts + i;
  }

  return NULL;
}

Chart* SONG_findChartByNumericLevelIndex(Song* song, u8 levelIndex) {
  if (levelIndex < song->chartCount)
    return song->charts + levelIndex;

  return NULL;
}

void SONG_free(Song* song) {
  free(song->title);
  free(song->artist);

  if (song->hasMessage)
    free(song->message);

  for (u32 i = 0; i < song->chartCount; i++) {
    if ((song->charts + i)->eventCount > 0)
      free((song->charts + i)->events);
  }
  free(song->charts);

  delete song;
}
