#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include "objects/ArrowEnums.h"
#include "utils/pool/ObjectPool.h"

const int HOLD_ARROW_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_TAIL_OFFSETS[] = {7, 8, 8, 8, 7};
const int HOLD_CACHE_MISS = -99;

class HoldArrow : public IPoolable {
 public:
  u32 id;
  ArrowDirection direction;
  int startTime;
  int endTime;
  int fillOffsetTop = 0;
  int fillOffsetBottom;
  u32 activeFillCount;
  int currentFillOffset = 0;
  int cachedHeadY = HOLD_CACHE_MISS;
  int cachedTailY = HOLD_CACHE_MISS;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}

  inline void resetState() {
    currentFillOffset = fillOffsetTop;
    cachedHeadY = HOLD_CACHE_MISS;
    cachedTailY = HOLD_CACHE_MISS;
  }

  u32 getFillSectionLength(int headY) {
    return fillOffsetBottom - (headY + fillOffsetTop);
  }

  template <typename F>
  inline int getHeadY(F get) {
    if (cachedHeadY == HOLD_CACHE_MISS)
      cachedHeadY = get();

    return cachedHeadY;
  }

  template <typename F>
  inline int getTailY(F get) {
    if (cachedTailY == HOLD_CACHE_MISS)
      cachedTailY = get();

    return cachedTailY;
  }
};

#endif  // HOLD_ARROW_H
