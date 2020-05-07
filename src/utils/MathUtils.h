#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/gba_engine.h>

const int MATH_PRECISION = 1000;
const int MATH_HALF_PRECISION = 500;

inline int MATH_roundingDiv(s32 numerator, s32 denominator) {
  int result = Div(numerator, denominator);
  int decimalPart =
      abs(DivMod(Div(numerator * MATH_PRECISION, denominator), MATH_PRECISION));
  int sign = result >= 0 ? 1 : -1;

  return decimalPart > MATH_HALF_PRECISION ? result + sign : result;
}

#endif  // MATH_UTILS_H
