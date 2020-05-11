#ifndef JUDGE_H
#define JUDGE_H

#include <functional>

#include "gameplay/TimingProvider.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

const u32 TIMING_WINDOW = 9;
const u32 OFFSET_BAD = 7;
const u32 OFFSET_GOOD = 5;
const u32 OFFSET_GREAT = 3;
const int HOLD_ARROW_TICK_OFFSET_MS = 84;
//                                    ^ OFFSET_GOOD * FRAME_MS = 5 * 16.73322954
const u32 FRAME_MS = 17;

class Judge {
 public:
  Judge(ObjectPool<Arrow>* arrowPool,
        std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders,
        Score* score,
        std::function<void()> onStageBreak);

  void onPress(Arrow* arrow, TimingProvider* timingProvider, int offset);
  void onOut(Arrow* arrow);
  void onHoldTick(u8 arrows, bool canMiss);

  inline bool isInsideTimingWindow(u32 diff) {
    return diff < FRAME_MS * TIMING_WINDOW;
  }

 private:
  ObjectPool<Arrow>* arrowPool;
  std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders;
  Score* score;
  std::function<void()> onStageBreak;

  FeedbackType onResult(Arrow* arrow, FeedbackType partialResult);
  void updateScore(FeedbackType result);
};

#endif  // JUDGE_H
