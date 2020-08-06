#include "PixelBlink.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "EffectUtils.h"

PixelBlink::PixelBlink(u32 targetValue) {
  this->targetValue = targetValue;
}

void PixelBlink::blink() {
  step = 0;
  isBlinking = true;
}

void PixelBlink::blinkAndThen(std::function<void()> callback) {
  blink();
  this->callback = callback;
}

void PixelBlink::tick(u8 minValue) {
  if (!isBlinking && step == 0)
    return;

  if (isBlinking) {
    step++;
    if (step == targetValue) {
      isBlinking = false;
      if (callback != NULL) {
        callback();
        callback = NULL;
      }
    }
  } else
    step--;

  EFFECT_setMosaic(max(step, minValue));
}
