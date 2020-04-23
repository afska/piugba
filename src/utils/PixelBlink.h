#ifndef PIXEL_BLINK_H
#define PIXEL_BLINK_H

#include <libgba-sprite-engine/gba/tonc_core.h>

class PixelBlink {
 public:
  PixelBlink();

  void blink();

  void tick();

 private:
  u32 step = 0;
  bool isBlinking = false;
};

#endif  // PIXEL_BLINK_H
