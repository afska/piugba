#include "ChartReader.h"

// TODO: Unhardcode offset (header), tempo (SET_TEMPO), speed (SET_SPEED),
// anticipation (look-up table: 3=870, 4=653, etc).
const u32 BPM = 162;
const int OFFSET = 175;
const int EMULATOR_MGBA_LAG = 150;
/*
  x = x0 + v * t
  ARROW_CORNER_MARGIN = GBA_SCREEN_HEIGHT + ARROW_SPEED * t
  t = (ARROW_CORNER_MARGIN - GBA_SCREEN_HEIGHT) px / ARROW_SPEED px/frame
  t = (4 - 160) / 3 = -52 frames * 16.73322954 ms/frame = -870,12793608 frames
*/
const int ANTICIPATION = 870 - EMULATOR_MGBA_LAG;

ChartReader::ChartReader(Chart* chart) {
  this->chart = chart;
};

bool ChartReader::update(u32 msecs, ObjectQueue<Arrow>* arrowQueue) {
  processNextEvent(msecs, arrowQueue);
  return animateBpm(msecs);
};

bool ChartReader::animateBpm(u32 msecs) {
  int msecsWithOffset = msecs - OFFSET;

  // 60000 ms           -> BPM beats
  // msecsWithOffset ms -> x = millis * BPM / 60000
  int beat = Div(msecsWithOffset * BPM, 60000);
  bool hasChanged = beat != lastBeat;
  lastBeat = beat;

  return hasChanged;
}

void ChartReader::processNextEvent(u32 msecs, ObjectQueue<Arrow>* arrowQueue) {
  while (msecs >= chart->events[eventIndex].timestamp - ANTICIPATION &&
         eventIndex < chart->eventCount) {
    auto event = chart->events[eventIndex];
    EventType type = static_cast<EventType>((event.data & EVENT_ARROW_TYPE));

    switch (type) {
      case EventType::NOTE:
        processNote(event.data, arrowQueue);
        break;
      default:
        break;
    }

    eventIndex++;
  }
}

void ChartReader::processNote(u8 data, ObjectQueue<Arrow>* arrowQueue) {
  if (data & EVENT_ARROW_DOWNLEFT)
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNLEFT); });
  if (data & EVENT_ARROW_UPLEFT)
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPLEFT); });
  if (data & EVENT_ARROW_CENTER)
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::CENTER); });
  if (data & EVENT_ARROW_UPRIGHT)
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPRIGHT); });
  if (data & EVENT_ARROW_DOWNRIGHT)
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNRIGHT); });
}
