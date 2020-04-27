#include "PixelBlink.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "EffectUtils.h"

const u32 TARGET_VALUE = 6;

PixelBlink::PixelBlink() {}

void PixelBlink::blink() {
  step = 0;
  isBlinking = true;
}

void PixelBlink::blinkAndThen(std::function<void()> callback) {
  blink();
  this->callback = callback;
}

void PixelBlink::tick() {
  if (isBlinking) {
    step++;
    if (step == TARGET_VALUE) {
      isBlinking = false;
      if (callback != NULL) {
        callback();
        callback = NULL;
      }
    }
  } else
    step = max(step - 1, 0);

  EFFECT_setMosaic(step);
}
