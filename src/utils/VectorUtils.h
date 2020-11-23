#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <vector>

inline bool VECTOR_contains(std::vector<u8> numbers, u8 number) {
  for (auto& it : numbers)
    if (it == number)
      return true;

  return false;
}

#endif  // VECTOR_UTILS_H
