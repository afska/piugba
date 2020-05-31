#ifndef ARROW_ENUMS_H
#define ARROW_ENUMS_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba_engine.h>

// TEST MACROS ---
#define TEST_MODE true
#define KEYTEST_MODE false
#define TIMINGTEST_MODE false
#define IFTEST if (TEST_MODE)
#define IFNOTTEST if (!TEST_MODE)
#define IFKEYTEST if (KEYTEST_MODE)
#define IFTIMINGTEST if (TIMINGTEST_MODE)
#define IFNOTKEYTEST if (!KEYTEST_MODE)
#define IFNOTTIMINGTEST if (!TIMINGTEST_MODE)
#define LOGN(NUM, LINE) (LOGSTR(std::to_string(NUM).c_str(), LINE))
#define LOGSTR(STR, LINE) (TextStream::instance().setText(STR, 1 + LINE, 15))
#define DEBULOG(NUM) LOGN(NUM, -1)
static int DEBULIST_LINE = -1;
inline void DEBULIST(int num) {
  LOGN(num, DEBULIST_LINE);
  DEBULIST_LINE++;
}
#include <libgba-sprite-engine/background/text_stream.h>
// ---------------

const u32 ARROWS_TOTAL = 5;
const u32 ARROW_FRAMES = 10;
const int ARROW_OFFSCREEN_LIMIT = -13;
const u32 ARROW_CORNER_MARGIN_X = 4;
const u32 ARROW_TILEMAP_LOADING_ID = 1000;
const u32 ARROW_LAYER_FRONT = 0;
const u32 ARROW_LAYER_MIDDLE = 1;
const u32 ARROW_LAYER_BACK = 2;
const u32 ARROW_ANIMATION_FRAMES = 5;
const u32 ARROW_ANIMATION_DELAY = 2;
const u32 ARROW_HOLD_FILL_TILE = 9;
const u32 ARROW_HOLD_TAIL_TILE = 0;

const u32 ARROW_DEFAULT_MULTIPLIER = 3;
const u32 ARROW_MIN_MULTIPLIER = 1;
const u32 ARROW_MAX_MULTIPLIER = 4;
const u32 ARROW_SIZE = 16;
const u32 ARROW_HALF_SIZE = 8;
const u32 ARROW_QUARTER_SIZE = 4;
const u32 ARROW_MARGIN = ARROW_SIZE + 2;
const u32 ARROW_INITIAL_Y = GBA_SCREEN_HEIGHT;
const u32 ARROW_FINAL_Y = 15;
const u32 ARROW_DISTANCE = ARROW_INITIAL_Y - ARROW_FINAL_Y;

enum ArrowType { UNIQUE, HOLD_HEAD, HOLD_FILL, HOLD_TAIL, HOLD_FAKE_HEAD };
enum ArrowDirection { DOWNLEFT, UPLEFT, CENTER, UPRIGHT, DOWNRIGHT };
enum ArrowState { ACTIVE, OUT };

#endif  // ARROW_ENUMS_H
