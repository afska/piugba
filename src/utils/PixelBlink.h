#ifndef PIXEL_BLINK_H
#define PIXEL_BLINK_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <functional>

class PixelBlink {
 public:
  PixelBlink();

  void blink();
  void blinkAndThen(std::function<void()> callback);

  void tick();

 private:
  u32 step = 0;
  bool isBlinking = false;
  std::function<void()> callback = NULL;
};

#endif  // PIXEL_BLINK_H
