#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <stdint.h>

#define INFINITY (u32)0xFFFFFFFF

extern "C" {
uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));
}

// Frac is in [0, INFINITY]
// => x * 0.5 = fracumul(x, INFINITY * 0.5);
inline u32 MATH_fracumul(u32 x, u32 frac) {
  return frac == 1 ? x : fracumul(x, frac);
}

inline u32 MATH_div(u32 numerator, u32 denominator) {
  if (denominator == 0)
    return INFINITY;
  if (denominator == INFINITY)
    return 0;

  return Div(numerator, denominator);
}

inline u32 MATH_mul(u32 n1, u32 n2) {
  return n1 == INFINITY || n2 == INFINITY ? INFINITY : n1 * n2;
}

inline void MATH_approximate(u32* value, u32 targetValue, u32 maxJump) {
  if (*value < targetValue)
    *value += min(targetValue - *value, maxJump);
  else
    *value -= min(*value - targetValue, maxJump);
}

inline int MATH_divCeil(int numerator, int denominator) {
  int result = Div(numerator, denominator);
  return result * denominator < numerator ? result + 1 : result;
}

inline u8 MATH_max(u8 a, u8 b, u8 c) {
  u8 max = (b > a) ? b : a;
  return (c > max) ? c : max;
}

#endif  // MATH_UTILS_H
