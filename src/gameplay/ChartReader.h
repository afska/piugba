#ifndef CHART_READER_H
#define CHART_READER_H

#include <array>
#include <functional>
#include <vector>

#include "HoldArrow.h"
#include "Judge.h"
#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class ChartReader {
 public:
  bool hasStopped = false;
  std::unique_ptr<ObjectPool<HoldArrow>> holdArrows;

  ChartReader(Chart* chart, Judge* judge);

  bool update(u32* msecs, ObjectPool<Arrow>* arrowPool);

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
  u32 timeNeeded = 0;
  u32 eventIndex = 0;
  u32 bpm = 0;
  int lastBeat = -1;
  u32 lastBpmChange = 0;
  u32 tickCount = 4;
  int lastTick = 0;
  u32 stopStart = 0;
  u32 stopEnd = 0;

  bool animateBpm(int msecsWithOffset);
  void processNextEvent(u32 msecs, ObjectPool<Arrow>* arrowPool);
  void processUniqueNote(u8 data, ObjectPool<Arrow>* arrowPool);
  void startHoldNote(Event* event, ObjectPool<Arrow>* arrowPool);
  void endHoldNote(Event* event, ObjectPool<Arrow>* arrowPool);
  void processHoldArrows(u32 msecs, ObjectPool<Arrow>* arrowPool);
  void processHoldTicks(u32 msecs, int msecsWithOffset);
  void connectArrows(std::vector<Arrow*>& arrows);

  template <typename F>
  inline void forEachDirection(u8 data, F action) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      if (data & EVENT_ARROW_MASKS[i])
        action(static_cast<ArrowDirection>(i));
    }
  }
};

#endif  // CHART_READER_H
