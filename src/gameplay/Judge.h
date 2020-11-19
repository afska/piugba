#ifndef JUDGE_H
#define JUDGE_H

#include <array>
#include <functional>
#include <vector>

#include "gameplay/TimingProvider.h"
#include "gameplay/save/SaveFile.h"
#include "objects/Arrow.h"
#include "objects/ArrowHolder.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

const u32 TIMING_WINDOWS[] = {0, 2, 4, 6, 8};
const int HOLD_ARROW_TICK_OFFSET_MS = 67;
//                                    ^ OFFSET_GOOD * FRAME_MS = 4 * 16.73322954

class Judge {
 public:
  Judge(ObjectPool<Arrow>* arrowPool,
        std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders,
        std::array<std::unique_ptr<Score>, GAME_MAX_PLAYERS>* scores,
        std::function<void(u8 playerId)> onStageBreak);

  void onPress(Arrow* arrow, TimingProvider* timingProvider, int offset);
  void onOut(Arrow* arrow);
  void onHoldTick(u8 arrows, u8 playerId, bool canMiss);

  inline void disable() { isDisabled = true; }
  inline void enable() { isDisabled = false; }

  inline bool isInsideTimingWindow(u32 diff) {
    return diff < FRAME_MS * getTimingWindowOf(FeedbackType::MISS);
  }

  inline bool isPressed(ArrowDirection direction, u8 playerId) {
    return arrowHolders->at(playerId * ARROWS_TOTAL + direction)
        ->getIsPressed();
  }

 private:
  ObjectPool<Arrow>* arrowPool;
  std::vector<std::unique_ptr<ArrowHolder>>* arrowHolders;
  std::array<std::unique_ptr<Score>, GAME_MAX_PLAYERS>* scores;
  std::function<void(u8 playerId)> onStageBreak;
  bool isDisabled = false;

  inline u32 getTimingWindowOf(FeedbackType feedbackType) {
    return TIMING_WINDOWS[feedbackType];
  }

  FeedbackType onResult(Arrow* arrow, FeedbackType partialResult);
  void updateScore(FeedbackType result, u8 playerId, bool isLong = false);
};

#endif  // JUDGE_H
