#include "ChartReader.h"

ChartReader::ChartReader(Chart* chart) {
  this->chart = chart;
};

void ChartReader::update(u32 msecs, ObjectQueue<Arrow>* arrowQueue) {
  while (msecs >= chart->events[eventIndex].timestamp &&
         eventIndex < chart->length) {
    auto event = chart->events[eventIndex];
    EventType type = static_cast<EventType>((event.data & EVENT_ARROW_TYPE));

    switch (type) {
      case EventType::NOTE:
        processNote(event.data, arrowQueue);
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
