#ifndef EVALUATION_H
#define EVALUATION_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "objects/score/Grade.h"
#include "utils/MathUtils.h"

const u32 FRACUMUL_0_05 = 214748365;
const u32 FRACUMUL_0_20 = 858993459;
const u32 FRACUMUL_0_45 = 1932735283;
const u32 FRACUMUL_0_60 = 2576980377;
const u32 FRACUMUL_0_90 = 3865470565;

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
    u32 percent = Div(max(perfects + MATH_fracumul(perfects, FRACUMUL_0_20) +
                              MATH_fracumul(greats, FRACUMUL_0_90) +
                              MATH_fracumul(goods, FRACUMUL_0_60) -
                              MATH_fracumul(bads, FRACUMUL_0_45) -
                              MATH_fracumul(misses, FRACUMUL_0_90) -
                              MATH_fracumul(longNotes, FRACUMUL_0_20) +
                              MATH_fracumul(maxCombo, FRACUMUL_0_05),
                          0) *
                          100,
                      totalNotes());

    if (percent >= 95 && misses == 0)
      return GradeType::S;
    else if (percent >= 95)
      return GradeType::A;
    else if (percent >= 90)
      return GradeType::B;
    else if (percent >= 85)
      return GradeType::C;
    else if (percent >= 75)
      return GradeType::D;
    else
      return GradeType::F;
  }

  inline u32 totalNotes() { return perfects + greats + goods + bads + misses; }
};

#endif  // EVALUATION_H
