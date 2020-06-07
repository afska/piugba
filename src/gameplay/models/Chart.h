#ifndef CHART_H
#define CHART_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Event.h"

enum DifficultyLevel { NORMAL, HARD, CRAZY, NUMERIC };

typedef struct {
  DifficultyLevel difficulty;  // u8
  u8 level;                    // 0-30

  u32 eventCount;
  Event* events;  // ("eventCount" times)
} Chart;

#endif  // CHART_H
