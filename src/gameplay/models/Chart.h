#ifndef CHART_H
#define CHART_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "Event.h"

enum DifficultyLevel { NORMAL, HARD, CRAZY, NUMERIC };
enum ChartType { SINGLE_CHART, DOUBLE_CHART, DOUBLE_COOP_CHART };
const u32 MAX_DIFFICULTY = 2;

typedef struct {
  DifficultyLevel difficulty;  // u8
  u8 level;                    // (0~99)
  char variant;    // '\0' or 'a', 'b', 'c', etc. for repeated levels
  ChartType type;  // u8

  u32 eventChunkSize;

  u32 rythmEventCount;
  Event* rythmEvents;  // ("rythmEventCount" times)

  u32 eventCount;
  Event* events;  // ("eventCount" times)

  bool isDouble;  // type == ChartType::DOUBLE_CHART || type ==
                  // ChartType::DOUBLE_COOP_CHART
} Chart;

#endif  // CHART_H
