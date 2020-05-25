#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include "objects/Arrow.h"
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
  u32 fillCount;
  int cachedStartY = HOLD_CACHE_MISS;
  int cachedEndY = HOLD_CACHE_MISS;
  Arrow* lastFill;
  Arrow* tail;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}

  inline void resetCache() {
    cachedStartY = HOLD_CACHE_MISS;
    cachedEndY = HOLD_CACHE_MISS;
  }

  template <typename F>
  inline int getStartY(F get) {
    if (cachedStartY == HOLD_CACHE_MISS)
      cachedStartY = get();

    return cachedStartY;
  }

  template <typename F>
  inline int getEndY(F get) {
    if (cachedEndY == HOLD_CACHE_MISS)
      cachedEndY = get();

    return cachedEndY;
  }
};

#endif  // HOLD_ARROW_H
