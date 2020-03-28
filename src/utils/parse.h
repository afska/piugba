#ifndef PARSE_H
#define PARSE_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <stdlib.h>
#include <string.h>

inline u32 as_u32le(u8* data) {
  return *data + (*(data + 1) << 8) + (*(data + 2) << 16) + (*(data + 3) << 24);
}

inline u8* parse_array(u8* source, u32* cursor, void* target, u32 length) {
  u8* data = source + *cursor;
  memcpy(target, data, length);
  *cursor += length;

  return data;
}

inline u8 parse_u8(u8* source, u32* cursor) {
  u8* data = source + *cursor;
  *cursor += sizeof(u8);

  return *data;
}

inline u32 parse_u32le(u8* source, u32* cursor) {
  u8* data = source + *cursor;
  *cursor += sizeof(u32);

  return as_u32le(data);
}

#endif  // PARSE_H
