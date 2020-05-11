#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

class HoldArrow : public IPoolable {
 public:
  u32 id;
  ArrowDirection direction;
  int startTime;
  int endTime;
  int fillCount;
  Arrow* lastFill;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}
};

#endif  // HOLD_ARROW_H
