#include "ChartReader.h"

/*
  x = x0 + v * t
  ARROW_CORNER_MARGIN = GBA_SCREEN_HEIGHT + ARROW_SPEED * t
  t = (ARROW_CORNER_MARGIN - GBA_SCREEN_HEIGHT) px / SPEED px/frame
  t = (4 - 160) / 3 = -52 frames * 16.73322954 ms/frame = -870,12793608 frames
*/
int TIME_BEFOREHAND = 870;

ChartReader::ChartReader(Chart* chart) {
  this->chart = chart;
};

void ChartReader::update(u32 msecs, ObjectQueue<Arrow>* arrowQueue) {
  // TODO: Unhardcode speed, time offset, etc

  while (msecs >= chart->events[eventIndex].timestamp - TIME_BEFOREHAND &&
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
};

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
