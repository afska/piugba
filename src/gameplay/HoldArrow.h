#ifndef HOLD_ARROW_H
#define HOLD_ARROW_H

#include "objects/Arrow.h"
#include "utils/pool/ObjectPool.h"

const int HOLD_ARROW_FILL_OFFSETS[] = {8, 5, 2, 5, 8};
const int HOLD_ARROW_TAIL_OFFSETS[] = {7, 8, 8, 8, 7};

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

  inline bool isLeftover(Arrow* arrow) {
    return tail != NULL && arrow->get()->getY() > tail->get()->getY();
  }

  inline bool needsFillsAtTheEnd() {
    return endTime == 0 &&
           lastFill->get()->getY() < (int)(ARROW_INITIAL_Y - ARROW_SIZE);
  }

  inline bool needsFillsInTheMiddle() {
    return tail != NULL && !SPRITE_isHidden(tail->get()) &&
           tail->get()->getY() - lastFill->get()->getY() > (int)ARROW_SIZE;
  }
};

#endif  // HOLD_ARROW_H
