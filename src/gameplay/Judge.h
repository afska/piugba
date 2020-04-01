#ifndef JUDGE_H
#define JUDGE_H

#include "objects/Arrow.h"
#include "objects/score/Score.h"
#include "utils/pool/ObjectQueue.h"

class Judge {
 public:
  Judge();

  void onPress(ArrowType arrowType,
               ObjectQueue<Arrow>* arrowQueue,
               Score* score);
};

#endif  // JUDGE_H
