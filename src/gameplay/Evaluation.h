#ifndef EVALUATION_H
#define EVALUATION_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"
#include "utils/MathUtils.h"

class Evaluation {
 public:
  Evaluation() {}

  u32 perfects = 0;
  u32 greats = 0;
  u32 goods = 0;
  u32 bads = 0;
  u32 misses = 0;

  u32 maxCombo = 0;
  u32 points = 0;
  u32 longNotes = 0;

  inline GradeType getGrade() {
    // TODO: IMPLEMENT

    return GradeType::S;
  }

  inline u32 totalNotes() { return perfects + greats + goods + bads + misses; }
};

#endif  // EVALUATION_H
