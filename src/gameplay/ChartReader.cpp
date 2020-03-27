#include "ChartReader.h"

ChartReader::ChartReader(Chart* chart) {
  this->chart = chart;
};

void ChartReader::update(u32 msecs,
                         std::unique_ptr<ObjectQueue<Arrow>> arrowQueue) {
  while (msecs >= chart->events[eventIndex].timestamp &&
         eventIndex < chart->length) {
    auto event = chart->events[eventIndex];
    EventType type = static_cast<EventType>((event.data & EVENT_ARROW_TYPE));

    switch (type) {
      case EventType::NOTE:
        ArrowType arrowType = getArrowType(event.data);
        arrowQueue->push(
            [&arrowType](Arrow* it) { it->initialize(arrowType); });
        break;
    }

    eventIndex++;
  }
};

ArrowType getArrowType(u8 data) {
  if (data & EVENT_ARROW_DOWNLEFT)
    return ArrowType::DOWNLEFT;
  if (data & EVENT_ARROW_UPLEFT)
    return ArrowType::UPLEFT;
  if (data & EVENT_ARROW_CENTER)
    return ArrowType::CENTER;
  if (data & EVENT_ARROW_UPRIGHT)
    return ArrowType::UPRIGHT;
  if (data & EVENT_ARROW_DOWNRIGHT)
    return ArrowType::DOWNRIGHT;

  return ArrowType::DOWNLEFT;
}
