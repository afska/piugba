#ifndef PIXEL_BLINK_H
#define PIXEL_BLINK_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <functional>

class PixelBlink {
 public:
  PixelBlink(u32 targetValue);

  void blink();
  void blinkAndThen(std::function<void()> callback);

  inline void tick() { tick(0); }
  void tick(u8 minValue);

 private:
  u32 targetValue;
  u32 step = 0;
  bool isBlinking = false;
  std::function<void()> callback = NULL;
};

#endif  // PIXEL_BLINK_H
