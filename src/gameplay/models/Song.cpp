#include "Song.h"

#include <string.h>

#include "utils/VectorUtils.h"

const u32 TITLE_LEN = 31;
const u32 ARTIST_LEN = 27;
const u32 MESSAGE_LEN = 107;

typedef struct {
  Event rythmEvents[1000];
  Event events[3000];
} ChartAllocation;  // (96000 bytes)

__attribute__((section(".ewram"))) ChartAllocation chartAllocations[2];

void parseEvents(Event* events,
                 u32 count,
                 bool isDouble,
                 u8* data,
                 u32* cursor);

Song* SONG_parse(const GBFS_FILE* fs,
                 SongFile* file,
                 std::vector<u8> chartIndexes) {
  u32 length;
  auto data = (u8*)gbfs_get_obj(fs, file->getMetadataFile().c_str(), &length);

  u32 cursor = 0;
  auto song = new Song();

  song->id = parse_u8(data, &cursor);
  song->totalSize = 0;

  song->title = new (std::nothrow) char[TITLE_LEN];
  parse_array(data, &cursor, song->title, TITLE_LEN);

  song->artist = new (std::nothrow) char[ARTIST_LEN];
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
  song->bounce = parse_u8(data, &cursor);
  song->colorFilter = parse_u8(data, &cursor);
  song->speedHack = parse_u8(data, &cursor);
  song->hasMessage = parse_u8(data, &cursor);

  if (song->hasMessage) {
    song->message = new (std::nothrow) char[MESSAGE_LEN];
    parse_array(data, &cursor, song->message, MESSAGE_LEN);
  }

  song->chartCount = parse_u8(data, &cursor);
  song->charts = new (std::nothrow) Chart[song->chartCount];
  song->totalSize += sizeof(Chart) * song->chartCount;
  u32 slot = 0;
  for (u32 i = 0; i < song->chartCount; i++) {
    auto chart = song->charts + i;

    chart->difficulty = static_cast<DifficultyLevel>(parse_u8(data, &cursor));
    chart->level = parse_u8(data, &cursor);
    chart->variant = parse_u8(data, &cursor);
    chart->offsetLabel = parse_u8(data, &cursor);
    chart->type = static_cast<ChartType>(parse_u8(data, &cursor));
    chart->isDouble = chart->type == ChartType::DOUBLE_CHART ||
                      chart->type == ChartType::DOUBLE_COOP_CHART;
    chart->customOffset = 0;
    chart->levelIndex = 0;

    chart->eventChunkSize = parse_u32le(data, &cursor);
    bool shouldParseEvents = VECTOR_contains(chartIndexes, i);
    if (!shouldParseEvents) {
      cursor += chart->eventChunkSize;
      chart->rythmEventCount = 0;
      chart->eventCount = 0;
      continue;
    }

    chart->rythmEventCount = parse_u32le(data, &cursor);
    chart->rythmEvents = chartAllocations[slot].rythmEvents;
    song->totalSize += sizeof(Event) * chart->rythmEventCount;
    parseEvents(chart->rythmEvents, chart->rythmEventCount, chart->isDouble,
                data, &cursor);

    chart->eventCount = parse_u32le(data, &cursor);
    chart->events = chartAllocations[slot].events;
    song->totalSize += sizeof(Event) * chart->eventCount;
    parseEvents(chart->events, chart->eventCount, chart->isDouble, data,
                &cursor);
    slot++;
  }

  song->index = file->index;
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

  u32 cursor = sizeof(u8) + TITLE_LEN + ARTIST_LEN;
  auto channel = static_cast<Channel>(parse_u8(data, &cursor));

  if (gameMode != GameMode::CAMPAIGN)
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

u32 SONG_findChartIndexByDifficultyLevel(Song* song,
                                         DifficultyLevel difficultyLevel) {
  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].difficulty == difficultyLevel)
      return i;
  }

  return 0;
}

Chart* SONG_findChartByDifficultyLevel(Song* song,
                                       DifficultyLevel difficultyLevel) {
  u32 index = SONG_findChartIndexByDifficultyLevel(song, difficultyLevel);
  return song->charts + index;
}

u32 SONG_findChartIndexByNumericLevelIndex(Song* song,
                                           u8 numericLevelIndex,
                                           bool isDouble) {
  u32 currentIndex = 0;

  for (u32 i = 0; i < song->chartCount; i++) {
    if (song->charts[i].isDouble != isDouble)
      continue;

    if (currentIndex == numericLevelIndex)
      return i;

    currentIndex++;
  }

  return 0;
}

Chart* SONG_findChartByNumericLevelIndex(Song* song,
                                         u8 numericLevelIndex,
                                         bool isDouble) {
  u32 index =
      SONG_findChartIndexByNumericLevelIndex(song, numericLevelIndex, isDouble);
  return song->charts + index;
}

void SONG_free(Song* song) {
  delete[] song->title;
  delete[] song->artist;

  if (song->hasMessage)
    delete[] song->message;

  delete[] song->charts;

  delete song;
}

void parseEvents(Event* events,
                 u32 count,
                 bool isDouble,
                 u8* data,
                 u32* cursor) {
  for (u32 j = 0; j < count; j++) {
    auto event = events + j;

    u32 timestampAndData = parse_u32le(data, cursor);
    event->isFake = timestampAndData & 1;
    event->timestamp = (timestampAndData >> 1) & 0x7fffff;
    if (event->timestamp & 0x400000)  // (sign extension)
      event->timestamp |= 0xff800000;
    event->data = (timestampAndData >> 24) & 0xff;

    auto eventType = static_cast<EventType>(event->data & EVENT_TYPE);
    event->data2 =
        EVENT_HAS_DATA2(eventType, isDouble) ? parse_u8(data, cursor) : 0;

    if (EVENT_HAS_PARAM(eventType))
      event->param = parse_u32le(data, cursor);
    if (EVENT_HAS_PARAM2(eventType))
      event->param2 = parse_u32le(data, cursor);
    if (EVENT_HAS_PARAM3(eventType))
      event->param3 = parse_u32le(data, cursor);

    event->handled[0] = false;
    event->handled[1] = false;
  }
}
