#ifndef CHART_H
#define CHART_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include "Event.h"

enum Difficulty { NORMAL, HARD, CRAZY, NUMERIC };

typedef struct {
  Difficulty difficulty;  // u8
  u8 level;               // 0-30

  u32 length;
  Event* events;  // ("length" times)
} Chart;

#endif  // CHART_H
