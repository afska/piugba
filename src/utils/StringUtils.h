#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string.h>

#include <string>
#include <vector>

#define LINE_BREAK "\r\n"

inline std::vector<std::string> STRING_split(std::string str,
                                             std::string delimiter) {
  std::vector<std::string> output;

  auto start = 0U;
  auto end = str.find(delimiter);
  while (end != std::string::npos) {
    output.push_back(str.substr(start, end - start));
    start = end + delimiter.length();
    end = str.find(delimiter, start);
  }

  output.push_back(str.substr(start, end));

  return output;
}

inline std::string STRING_removeFromEnd(std::string str, int length) {
  return str.replace(str.length() - length, length, "");
}

inline bool STRING_endsWith(const char* str, const char* suffix) {
  if (!str || !suffix)
    return false;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return false;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

inline void STRING_padLeft(std::string& str, u32 num, char paddingChar = ' ') {
  if (num > str.size())
    str.insert(0, num - str.size(), paddingChar);
}

#endif  // STRING_UTILS_H
