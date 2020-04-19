#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string.h>

#include <string>

bool STRING_endsWith(const char* str, const char* suffix) {
  if (!str || !suffix)
    return false;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return false;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

std::string STRING_removeFromEnd(std::string str, int length) {
  return str.replace(str.length() - length, length, "");
}

#endif  // STRING_UTILS_H
