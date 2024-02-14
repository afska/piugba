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
  PixelTransitionEffect(){};

  void render() override {
    EFFECT_setMosaic(value);

    value++;

    if (isDone())
      SCENE_init();
  }

  bool isDone() override { return value >= TARGET_MOSAIC; }

 private:
  u32 value = 0;
};

#endif  // PIXEL_TRANSITION_EFFECT_H
