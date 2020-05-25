#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <libgba-sprite-engine/gba/tonc_bios.h>

inline int MATH_divCeil(int numerator, int denominator) {
  int result = Div(numerator, denominator);
  return result * denominator < numerator ? result + 1 : result;
}

#endif  // MATH_UTILS_H
