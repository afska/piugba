#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "objects/ArrowInfo.h"
#include "utils/pool/ObjectPool.h"

const int HOLD_ARROW_FIRST_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_LAST_FILL_OFFSETS[] = {7, 8, 8, 8, 7};
const int HOLD_OFFSCREEN_LIMIT = (int)-ARROW_SIZE * 2;
const int HOLD_FILL_FINAL_Y = ARROW_FINAL_Y + ARROW_SIZE;
const int HOLD_NULL = -999999999;

inline int HOLD_getFirstFillOffset(ArrowDirection direction) {
  return ARROW_SIZE - HOLD_ARROW_FIRST_FILL_OFFSETS[direction];
}

inline int HOLD_getLastFillOffset(ArrowDirection direction) {
  return -ARROW_SIZE + HOLD_ARROW_LAST_FILL_OFFSETS[direction];
}

class HoldArrow : public IPoolable {
 public:
  u32 id;
  ArrowDirection direction;
  int startTime;
  int endTime;
  int fillOffsetSkip = 0;
  int fillOffsetBottom;
  u32 activeFillCount;
  int lastPressTopY;
  int currentFillOffset = 0;
  int cachedHeadY = HOLD_NULL;
  int cachedTailY = HOLD_NULL;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}

  inline u32 getFillSectionLength(int topY, int bottomY) {
    return max(bottomY - (topY + fillOffsetSkip), 0);
  }

  inline bool hasEnded() { return endTime > 0; }

  inline void updateLastPress(int topY) { lastPressTopY = topY; }

  inline void resetState() {
    currentFillOffset = fillOffsetSkip;
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
