#ifndef CHART_H
#define CHART_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Event.h"

enum Difficulty { NORMAL, HARD, CRAZY, NUMERIC };

typedef struct {
  int offset;             // u8 (1=positive, 0=negative) + u32
  Difficulty difficulty;  // u8
  u8 level;               // 0-30

  u32 eventCount;
  Event* events;  // ("eventCount" times)
} Chart;

#endif  // CHART_H
