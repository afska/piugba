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
  Arrow* tail;

  HoldArrow(u32 id) { this->id = id; }
  void discard() override {}

  inline bool needsFillsAtTheEnd() {
    return endTime == 0 &&
           lastFill->get()->getY() < (int)(ARROW_INITIAL_Y - ARROW_SIZE);
  }

  inline bool needsFillsInTheMiddle() {
    return tail != NULL &&
           lastFill->get()->getY() + (int)ARROW_SIZE < tail->get()->getY();
  }
};

#endif  // HOLD_ARROW_H
