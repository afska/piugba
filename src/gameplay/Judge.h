#ifndef JUDGE_H
#define JUDGE_H

#include <functional>

#include "objects/Arrow.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

class Judge {
 public:
  Judge(ObjectPool<Arrow>* arrowPool,
        Score* score,
        std::function<void()> onStageBreak);

  void onPress(Arrow* arrow);
  void onOut(Arrow* arrow);
  void onHoldTick(ArrowDirection direction);

 private:
  ObjectPool<Arrow>* arrowPool;
  Score* score;
  std::function<void()> onStageBreak;

  FeedbackType onResult(Arrow* arrow, FeedbackType partialResult);
};

#endif  // JUDGE_H
