#ifndef CHART_READER_H
#define CHART_READER_H

#include <vector>

#include "HoldArrow.h"
#include "Judge.h"
#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class ChartReader {
 public:
  u32 timeNeeded = 0;
  bool hasStopped = false;

  ChartReader(Chart* chart, Judge* judge);

  bool preUpdate(int* msecs, ObjectPool<Arrow>* arrowPool);
  void postUpdate(int msecs, ObjectPool<Arrow>* arrowPool);

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

 private:
  Chart* chart;
  Judge* judge;
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

  bool animateBpm(int rythmMsecs);
  void processNextEvents(int* msecs, ObjectPool<Arrow>* arrowPool);
  void predictNoteEvents(int msecs, ObjectPool<Arrow>* arrowPool);
  void processUniqueNote(Event* event, ObjectPool<Arrow>* arrowPool);
  void startHoldNote(Event* event, ObjectPool<Arrow>* arrowPool);
  void endHoldNote(Event* event, ObjectPool<Arrow>* arrowPool);
  void processHoldArrows(int msecs, ObjectPool<Arrow>* arrowPool);
  void processHoldTicks(int msecs, int rythmMsecs);
  void connectArrows(std::vector<Arrow*>& arrows);
  void snapClosestArrowToHolder(int msecs, ObjectPool<Arrow>* arrowPool);
  void logDebugInfo(int msecs, ObjectPool<Arrow>* arrowPool);

  template <typename F>
  inline void forEachDirection(u8 data, F action) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      if (data & EVENT_ARROW_MASKS[i])
        action(static_cast<ArrowDirection>(i));
    }
  }
};

#endif  // CHART_READER_H
