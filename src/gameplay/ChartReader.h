#ifndef CHART_READER_H
#define CHART_READER_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba/tonc_math.h>

#include <vector>

#include "HoldArrow.h"
#include "Judge.h"
#include "TimingProvider.h"
#include "models/Song.h"
#include "objects/Arrow.h"
#include "utils/MathUtils.h"
#include "utils/pool/ObjectPool.h"

class ChartReader : public TimingProvider {
 public:
  ChartReader(Chart* chart, ObjectPool<Arrow>*, Judge* judge);

  bool update(int msecs);

  int getYFor(Arrow* arrow);

  inline u32 getMultiplier() { return multiplier; }
  inline void setMultiplier(u32 multiplier) {
    this->multiplier =
        max(min(multiplier, ARROW_MAX_MULTIPLIER), ARROW_MIN_MULTIPLIER);
    syncScrollSpeed();
  }

  bool isHoldActive(ArrowDirection direction);
  bool hasJustStopped();
  bool isAboutToResume();

  template <typename DEBUG>
  void logDebugInfo();

 private:
  Chart* chart;
  ObjectPool<Arrow>* arrowPool;
  Judge* judge;
  u32 targetArrowTime;
  u32 multiplier;
  u32 scrollFactor;
  std::unique_ptr<ObjectPool<HoldArrow>> holdArrows;
  std::array<HoldArrowState, ARROWS_TOTAL> holdArrowStates;
  int displayMsecs = 0;
  u32 eventIndex = 0;
  u32 subtick = 0;
  u32 bpm = 0;
  int lastBpmChange = 0;
  u32 tickCount = 4;
  int lastTick = 0;
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

    if (max != NULL &&
        max->startTime == holdArrowStates[direction].lastStartTime)
      action(max);
  }

  template <typename F>
  inline void forEachDirection(u8 data, F action) {
    for (u32 i = 0; i < ARROWS_TOTAL; i++) {
      if (data & EVENT_ARROW_MASKS[i])
        action(static_cast<ArrowDirection>(i));
    }
  }

  inline void syncScrollSpeed() {
    targetArrowTime =
        MATH_div(MINUTE * ARROW_SCROLL_LENGTH_BEATS, MATH_mul(bpm, multiplier));
  }
  inline void syncArrowTime() { arrowTime = targetArrowTime; }

  int getYFor(int timestamp);
  void processNextEvents();
  void predictNoteEvents();
  void processUniqueNote(Event* event);
  void startHoldNote(Event* event);
  void endHoldNote(Event* event);
  void orchestrateHoldArrows();
  bool processTicks(int rythmMsecs, bool checkHoldArrows);
  void connectArrows(std::vector<Arrow*>& arrows);
  int getFillTopY(HoldArrow* holdArrow);
  int getFillBottomY(HoldArrow* holdArrow, int topY);
};

class CHART_DEBUG;

#endif  // CHART_READER_H
