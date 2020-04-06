#include "ChartReader.h"

// TODO: Unhardcode offset (header), tempo (SET_TEMPO), speed (SET_SPEED),
// anticipation (look-up table: 3=870, 4=653, etc).
const u32 BPM = 162;
const int EMULATOR_AUDIO_LAG = 170;
/*
  x = x0 + v * t
  ARROW_CORNER_MARGIN_Y = GBA_SCREEN_HEIGHT + ARROW_SPEED * t
  t = (ARROW_CORNER_MARGIN_Y - GBA_SCREEN_HEIGHT) px / ARROW_SPEED px/frame
  t = (15 - 160) / 3 = -48.33 frames * 16.73322954 ms/frame = -808,77 frames
*/
const int ANTICIPATION = 809 - EMULATOR_AUDIO_LAG;

ChartReader::ChartReader(Chart* chart) {
  this->chart = chart;
};

bool ChartReader::update(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  processNextEvent(msecs, arrowPool);
  return animateBpm(msecs);
};

bool ChartReader::animateBpm(u32 msecs) {
  int msecsWithOffset = msecs - chart->offset;

  // 60000 ms           -> BPM beats
  // msecsWithOffset ms -> x = millis * BPM / 60000
  int beat = Div(msecsWithOffset * BPM, 60000);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool) {
  while (msecs >= chart->events[eventIndex].timestamp - ANTICIPATION &&
         eventIndex < chart->eventCount) {
    auto event = chart->events[eventIndex];
    EventType type = static_cast<EventType>((event.data & EVENT_ARROW_TYPE));

    switch (type) {
      case EventType::NOTE:
        processNote(event.data, arrowPool);
        break;
      default:
        break;
    }

    eventIndex++;
  }
}

void ChartReader::processNote(u8 data, ObjectPool<Arrow>* arrowPool) {
  std::vector<Arrow*> arrows;

  if (data & EVENT_ARROW_DOWNLEFT)
    arrows.push_back(arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::DOWNLEFT); }));
  if (data & EVENT_ARROW_UPLEFT)
    arrows.push_back(arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::UPLEFT); }));
  if (data & EVENT_ARROW_CENTER)
    arrows.push_back(arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::CENTER); }));
  if (data & EVENT_ARROW_UPRIGHT)
    arrows.push_back(arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::UPRIGHT); }));
  if (data & EVENT_ARROW_DOWNRIGHT)
    arrows.push_back(arrowPool->create(
        [](Arrow* it) { it->initialize(ArrowType::DOWNRIGHT); }));

  if (arrows.size() > 1) {
    for (u32 i = 0; i < arrows.size(); i++) {
      arrows[i]->setSiblingId(arrows[i == arrows.size() - 1 ? 0 : i + 1]->id);
    }
  }
}
