#ifndef JUDGE_H
#define JUDGE_H

#include <functional>

#include "gameplay/TimingProvider.h"
#include "gameplay/save/SaveFile.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

const u32 TIMING_WINDOWS[] = {0, 2, 4, 6, 8, 0, 2, 3, 4, 5};
const u32 EXTRA_JUDGEMENT_START = 5;
const int HOLD_ARROW_TICK_OFFSET_MS = 67;
//                                    ^ OFFSET_GOOD * FRAME_MS = 4 * 16.73322954

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
    return diff < FRAME_MS * getTimingWindowOf(FeedbackType::MISS);
  }

  inline bool isPressed(ArrowDirection direction) {
    return arrowHolders->at(direction)->getIsPressed();
  }

 private:
  ObjectPool<Arrow>* arrowPool;
  std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders;
  Score* score;
  std::function<void()> onStageBreak;

  inline u32 getTimingWindowOf(FeedbackType feedbackType) {
    return TIMING_WINDOWS[GameState.mods.extraJudgement *
                              EXTRA_JUDGEMENT_START +
                          feedbackType];
  }

  FeedbackType onResult(Arrow* arrow, FeedbackType partialResult);
  void updateScore(FeedbackType result, bool isLong);
};

#endif  // JUDGE_H
