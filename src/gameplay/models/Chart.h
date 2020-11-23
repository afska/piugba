#ifndef CHART_H
#define CHART_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Event.h"

enum DifficultyLevel { NORMAL, HARD, CRAZY, NUMERIC };
const u32 MAX_DIFFICULTY = 2;

typedef struct {
  DifficultyLevel difficulty;  // u8
  u8 level;                    // (0~99)
  bool isDouble;

  u32 eventChunkSize;
  u32 eventCount;
  Event* events;  // ("eventCount" times)
} Chart;

#endif  // CHART_H
