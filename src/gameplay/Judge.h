#ifndef JUDGE_H
#define JUDGE_H

#include "objects/Arrow.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectPool.h"

class Judge {
 public:
  Judge();

  void onPress(ArrowType arrowType, ObjectPool<Arrow>* arrowPool, Score* score);
};

#endif  // JUDGE_H
