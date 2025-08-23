#ifndef ARROW_ENUMS_H
#define ARROW_ENUMS_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba_engine.h>

#include "gameplay/debug/DebugTools.h"
#include "gameplay/save/SaveFile.h"

#define CODE_IWRAM __attribute__((section(".iwram"), target("arm"), noinline))

#define ARROWS_GAME_TOTAL (isDouble() ? 10 : 5)

const u32 ARROWS_TOTAL = 5;
const int ARROW_OFFSCREEN_LIMIT = 160;  // GBA_SCREEN_HEIGHT
const u32 ARROW_TILEMAP_LOADING_ID = 1000;
const u32 ARROW_LAYER_FRONT = 0;
const u32 ARROW_LAYER_MIDDLE = 1;
const u32 ARROW_LAYER_BACK = 2;

const u32 ARROW_MIN_MULTIPLIER = 1;
const u32 ARROW_MAX_MULTIPLIER = 6;
const u32 ARROW_SIZE = 16;
const u32 ARROW_HALF_SIZE = 8;
const u32 ARROW_QUARTER_SIZE = 4;
const u32 ARROW_MARGIN = ARROW_SIZE;
const u32 ARROW_INITIAL_Y = -16;
const u32 ARROW_CORNER_MARGIN_X_OFFSET = 4;

inline u32 ARROW_CORNER_MARGIN_X(u8 playerId) {
  return GameState.positionX[playerId] + ARROW_CORNER_MARGIN_X_OFFSET;
}

inline u32 ARROW_FINAL_Y() {
  return 134;  // GBA_SCREEN_HEIGHT - 26
}

inline u32 ARROW_DISTANCE() {
  return ARROW_FINAL_Y() - ARROW_INITIAL_Y;
}

enum ArrowType { UNIQUE, HOLD_HEAD, HOLD_FILL, HOLD_TAIL, HOLD_FAKE_HEAD };
enum ArrowDirection {
  DOWNLEFT,
  UPLEFT,
  CENTER,
  UPRIGHT,
  DOWNRIGHT,
  DOWNLEFT_DOUBLE,
  UPLEFT_DOUBLE,
  CENTER_DOUBLE,
  UPRIGHT_DOUBLE,
  DOWNRIGHT_DOUBLE
};
enum ArrowFlip { NO_FLIP, FLIP_X, FLIP_Y, FLIP_BOTH };

const u32 ARROW_ANIMATION_FRAMES = 5;
const u32 ARROW_ANIMATION_DELAY = 2;
const u8 ARROW_BASE_TILE[] = {0, 10, 20, 10, 0};
const u8 ARROW_END_TILE[] = {5, 5, 25, 5, 5};
const ArrowFlip ARROW_FLIP_TILE[] = {ArrowFlip::NO_FLIP, ArrowFlip::FLIP_Y,
                                     ArrowFlip::NO_FLIP, ArrowFlip::FLIP_BOTH,
                                     ArrowFlip::FLIP_X};
const u32 ARROW_HOLD_FILL_TILE = 9;
const u32 ARROW_HOLD_TAIL_TILE = 0;
const u32 ARROW_HOLD_FILL_FAKE_TILE = 30;
const u32 ARROW_FAKE_TILE = 2;

#endif  // ARROW_ENUMS_H
