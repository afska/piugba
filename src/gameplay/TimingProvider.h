#ifndef TIMING_PROVIDER_H
#define TIMING_PROVIDER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#ifndef AUDIO_LAG
// Defined in Makefile: Emulator's audio lag
#define AUDIO_LAG 0
#endif

/*
  x = x0 + v * t
  ARROW_FINAL_Y() = ARROW_INITIAL_Y + ARROW_SPEED * t
  t = abs(ARROW_INITIAL_Y - ARROW_FINAL_Y()) px / ARROW_SPEED px/frame
  t = (160 - 15) / 3 = (48.33 frames) * 16.73322954 ms/frame = 808,77 ms
  => Look-up table for speeds 0, 1, 2, 3, 4, 5 and 6 px/frame
*/
const u32 ARROW_TIME[] = {0, 2426, 1213, 809, 607, 485, 404};

const u32 MAX_ARROW_TIME_JUMP = 100;
const u32 MINUTE = 60000;
const u32 FRAME_MS = 17;
const u32 BEAT_UNIT = 4;
const u32 ARROW_SCROLL_LENGTH_BEATS = BEAT_UNIT * 2;
const u32 FRACUMUL_DIV_BY_MINUTE = 71583;  // (1/MINUTE) * INFINITY

class TimingProvider {
 public:
  inline int getMsecs() { return msecs; }
  inline u32 getArrowTime() { return arrowTime; }
  inline bool isStopped() { return hasStopped; }
  inline int getStopStart() { return stopStart; }
  inline u32 getStopLength() { return stopLength; }
  inline bool isStopJudgeable() { return stopJudgeable; }

 protected:
  int msecs = 0;
  bool hasStopped = false;
  u32 arrowTime;
  int stopStart = 0;
  u32 stopLength = 0;
  bool stopJudgeable = false;
};

#endif  // TIMING_PROVIDER_H
