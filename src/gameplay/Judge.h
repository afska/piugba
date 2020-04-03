#ifndef JUDGE_H
#define JUDGE_H

#include "objects/Arrow.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

class Judge {
 public:
  Judge(ObjectPool<Arrow>* arrowPool, Score* score);

  void onPress(Arrow* arrow);
  void onOut(Arrow* arrow);

 private:
  ObjectPool<Arrow>* arrowPool;
  Score* score;

  FeedbackType onResult(Arrow* arrow, FeedbackType partialResult);
};

#endif  // JUDGE_H
