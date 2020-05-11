#ifndef TIMING_PROVIDER_H
#define TIMING_PROVIDER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

// Emulator's audio lag
const int AUDIO_LAG = 180;

/*
  x = x0 + v * t
  ARROW_FINAL_Y = ARROW_INITIAL_Y + ARROW_SPEED * t
  t = abs(ARROW_INITIAL_Y - ARROW_FINAL_Y) px / ARROW_SPEED px/frame
  t = (160 - 15) / 3 = (48.33 frames) * 16.73322954 ms/frame = 792,03 ms
  => Look-up table for speeds 0, 1, 2, 3 and 4 px/frame
*/
const u32 TIME_NEEDED[] = {0, 2426, 1213, 809, 607};

const u32 MINUTE = 60000;

class TimingProvider {
 public:
  virtual int getMsecs() = 0;
  virtual u32 getTimeNeeded() = 0;
  virtual bool isStopped() = 0;
  virtual int getStopStart() = 0;
  virtual u32 getStopLength() = 0;
};

#endif  // TIMING_PROVIDER_H
