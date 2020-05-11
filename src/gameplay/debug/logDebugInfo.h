#ifndef LOG_DEBUG_INFO_H
#define LOG_DEBUG_INFO_H

#include "gameplay/ChartReader.h"

static u32 maxSprites = 0;

template <>
void ChartReader::logDebugInfo<CHART_DEBUG>() {
  Arrow* min = NULL;
  int minTimestamp = 0;
  u32 activeSprites = 0;
  arrowPool->forEachActive([&min, &minTimestamp, &activeSprites](Arrow* it) {
    activeSprites++;
    if (activeSprites > maxSprites)
      maxSprites = activeSprites;
    bool isActive = it->get()->getY() >= (int)ARROW_FINAL_Y;

    if (isActive && (min == NULL || it->timestamp < minTimestamp)) {
      min = it;
      minTimestamp = it->timestamp;
    }
  });

  LOGSTR("BPM:", 0);
  LOGN(bpm, 1);
  LOGSTR("MSECS:", 2);
  LOGN(msecs, 3);

  LOGSTR("NEXT -> MS:", 5);
  LOGN(min == NULL ? -1 : min->timestamp, 6);
  LOGSTR("NEXT -> Y:", 7);
  LOGN(min == NULL ? -1 : min->get()->getY() - (int)ARROW_FINAL_Y, 8);

  std::string typeStr;
  u32 currentIndex = eventIndex;
  while (currentIndex < chart->eventCount) {
    Event* event = chart->events + currentIndex;
    EventType type = static_cast<EventType>((event->data & EVENT_TYPE));
    if (EVENT_HAS_EXTRA(type))
      break;
    currentIndex++;
  }

  if (currentIndex < chart->eventCount) {
    Event* nextEvent = chart->events + currentIndex;
    EventType type = static_cast<EventType>((nextEvent->data & EVENT_TYPE));

    switch (type) {
      case EventType::SET_TEMPO:
        typeStr = "tempo";
        break;
      case EventType::SET_TICKCOUNT:
        typeStr = "tick";
        break;
      case EventType::STOP:
        typeStr = "stop";
        break;
      case EventType::WARP:
        typeStr = "warp";
        break;
      default:
        typeStr = std::to_string(type);
    }
    LOGSTR("EVENT:", 10);
    LOGSTR(typeStr, 11);
    LOGN(nextEvent->timestamp, 12);
    LOGSTR("-> " + std::to_string(nextEvent->extra), 13);
  }

  LOGSTR("SPRITES:", 15);
  LOGN(activeSprites, 16);
  LOGSTR("-> " + std::to_string(maxSprites), 17);
}

#endif  // LOG_DEBUG_INFO_H
