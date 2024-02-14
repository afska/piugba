#ifndef FADE_OUT_PIXEL_TRANSITION_EFFECT_H
#define FADE_OUT_PIXEL_TRANSITION_EFFECT_H

#include <libgba-sprite-engine/effects/scene_effect.h>
#include <libgba-sprite-engine/scene.h>

#include "utils/EffectUtils.h"
#include "utils/SceneUtils.h"

const u32 FINAL_OPACITY = 15;
const u32 FINAL_MOSAIC = 10;

void SCENE_init();

class FadeOutPixelTransitionEffect : public SceneEffect {
 public:
  FadeOutPixelTransitionEffect(){};

  void render() override {
    EFFECT_setBlendAlpha(opacity * 3 / 4);
    EFFECT_setMosaic(mosaic);

    if (opacity * 3 / 4 < FINAL_OPACITY)
      opacity++;
    else
      mosaic++;

    if (isDone())
      SCENE_init();
  }

  bool isDone() override { return mosaic >= FINAL_MOSAIC; }

 private:
  u32 opacity = 0;
  u32 mosaic = 0;
};

#endif  // FADE_OUT_PIXEL_TRANSITION_EFFECT_H
