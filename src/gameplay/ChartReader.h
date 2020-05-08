#ifndef CHART_READER_H
#define CHART_READER_H

#include <vector>

#include "HoldArrow.h"
#include "Judge.h"
#include "TimingProvider.h"
#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class ChartReader : public TimingProvider {
 public:
  ChartReader(Chart* chart, ObjectPool<Arrow>*, Judge* judge);

  int getMsecs() override { return msecs; }
  bool isStopped() override { return hasStopped; }

  bool preUpdate(int msecs);
  void postUpdate();

  int getYFor(int timestamp);
  int getTimestampFor(int y);

  bool isHoldActive(ArrowDirection direction);

 private:
  int msecs = 0;
  bool hasStopped = false;
  Chart* chart;
  ObjectPool<Arrow>* arrowPool;
  Judge* judge;
  u32 timeNeeded = 0;
  std::unique_ptr<ObjectPool<HoldArrow>> holdArrows;
  u32 eventIndex = 0;
  u32 bpm = 0;
  int lastBeat = -1;
  int lastBpmChange = 0;
  u32 tickCount = 4;
  int lastTick = 0;
  int stopStart = 0;
  u32 stopLength = 0;
  u32 stoppedMs = 0;
  u32 warpedMs = 0;

  template <typename F>
  inline void processEvents(int targetMsecs, F action) {
    u32 currentIndex = eventIndex;
    bool skipped = false;

    while (targetMsecs >= chart->events[currentIndex].timestamp &&
           currentIndex < chart->eventCount) {
      auto event = chart->events + currentIndex;
      event->index = currentIndex;
      EventType type = static_cast<EventType>((event->data & EVENT_TYPE));

      if (event->handled) {
        currentIndex++;
        continue;
      }

      bool stop = false;
      event->handled = action(type, event, &stop);
      currentIndex++;

      if (!event->handled)
        skipped = true;
      if (!skipped)
        eventIndex = currentIndex;
      if (stop)
        return;
    }
  }

  template <typename F>
  inline void withNextHoldArrow(ArrowDirection direction, F action) {
    HoldArrow* min = NULL;
    holdArrows->forEachActive(
        [&direction, &action, &min, this](HoldArrow* holdArrow) {
          if (holdArrow->direction != direction)
            return;

          if (min == NULL || holdArrow->startTime < min->startTime)
            min = holdArrow;
        });

    if (min != NULL)
      action(min);
  }

  template <typename F>
  inline void withLastHoldArrow(ArrowDirection direction, F action) {
    HoldArrow* max = NULL;
    holdArrows->forEachActive(
        [&direction, &action, &max, this](HoldArrow* holdArrow) {
          if (holdArrow->direction != direction)
            return;

          if (max == NULL || holdArrow->startTime > max->startTime)
            max = holdArrow;
        });

    if (max != NULL)
      action(max);
  }

  bool animateBpm(int rythmMsecs);
  void processNextEvents();
  void predictNoteEvents();
  void processUniqueNote(Event* event);
  void startHoldNote(Event* event);
  void endHoldNote(Event* event);
  void processHoldArrows();
  void processHoldTicks(int rythmMsecs);
  void connectArrows(std::vector<Arrow*>& arrows);
  void snapClosestArrowToHolder();

  template <typename DEBUG>
  void logDebugInfo();

  template <typename F>
  inline void forEachDirection(u8 data, F action) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      if (data & EVENT_ARROW_MASKS[i])
        action(static_cast<ArrowDirection>(i));
    }
  }
};

class CHART_DEBUG;

#endif  // CHART_READER_H
