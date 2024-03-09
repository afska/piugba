#ifndef PIXEL_TRANSITION_EFFECT_H
#define PIXEL_TRANSITION_EFFECT_H

#include <libgba-sprite-engine/effects/scene_effect.h>
#include <libgba-sprite-engine/scene.h>

#include "utils/EffectUtils.h"
#include "utils/SceneUtils.h"

const u32 TARGET_MOSAIC = 10;

void SCENE_init();

class PixelTransitionEffect : public SceneEffect {
 public:
  PixelTransitionEffect(u32 target = TARGET_MOSAIC) { this->target = target; };

  void render() override {
    EFFECT_setMosaic(value);

    value++;

    if (isDone())
      SCENE_init();
  }

  bool isDone() override { return value >= target; }

 private:
  u32 value = 0;
  u32 target = TARGET_MOSAIC;
};

#endif  // PIXEL_TRANSITION_EFFECT_H
