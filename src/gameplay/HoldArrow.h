#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "objects/ArrowInfo.h"
#include "utils/pool/ObjectPool.h"

// Depends on ARROWS_GAME_TOTAL
const int HOLD_ARROW_FIRST_FILL_OFFSETS[] = {8, 5, 2, 5, 8, 8, 5, 2, 5, 8};
const int HOLD_ARROW_LAST_FILL_OFFSETS[] = {7, 8, 8, 8, 7, 7, 8, 8, 8, 7};
const int HOLD_NULL = -999999999;

inline int HOLD_FILL_FINAL_Y() {
  return 134 - ARROW_HALF_SIZE;  // ARROW_FINAL_Y() - ARROW_HALF_SIZE
}

inline int HOLD_getFirstFillOffset(ArrowDirection direction) {
  return ARROW_SIZE - HOLD_ARROW_FIRST_FILL_OFFSETS[direction];
}

inline int HOLD_getLastFillOffset(ArrowDirection direction) {
  return -ARROW_SIZE + HOLD_ARROW_LAST_FILL_OFFSETS[direction];
}

typedef struct {
  bool isActive;
  int currentStartTime;
  int lastStartTime;
} HoldArrowState;

class HoldArrow : public IPoolable {
 public:
  u32 id;
  ArrowDirection direction;
  int startTime;
  int endTime;
  int fillOffsetSkip;
  int fillOffsetBottom;
  u32 activeFillCount;
  int lastPressTopY;
  bool isFake;
  int currentFillOffset = 0;
  int cachedHeadY = HOLD_NULL;
  int cachedTailY = HOLD_NULL;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}

  inline bool hasStarted(int msecs) { return msecs >= startTime; }
  inline bool hasEnded(int msecs) { return hasEndTime() && msecs >= endTime; }

  inline bool isOccurring(int msecs) {
    return hasStarted(msecs) && (!hasEndTime() || msecs < endTime);
  }

  inline bool hasEndTime() { return endTime > 0; }

  inline void updateLastPress(int topY) { lastPressTopY = topY; }

  inline u32 getFillSectionLength(int topY, int bottomY) {
    return max((topY - fillOffsetSkip) - bottomY, 0);
  }

  inline void resetState() {
    cachedHeadY = HOLD_NULL;
    cachedTailY = HOLD_NULL;
  }

  template <typename F>
  inline int getHeadY(F get) {
    if (cachedHeadY == HOLD_NULL)
      cachedHeadY = get();

    return cachedHeadY;
  }

  template <typename F>
  inline int getTailY(F get) {
    if (cachedTailY == HOLD_NULL)
      cachedTailY = get();

    return cachedTailY;
  }
};

#endif  // HOLD_ARROW_H
