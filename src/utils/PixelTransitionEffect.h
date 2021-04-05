#include <libgba-sprite-engine/effects/scene_effect.h>
#include <libgba-sprite-engine/scene.h>

#include "utils/EffectUtils.h"

const u32 TARGET_MOSAIC = 10;

class PixelTransitionEffect : public SceneEffect {
 public:
  PixelTransitionEffect(){};

  void update() override {
    EFFECT_setMosaic(value);
    value++;
  }

  bool isDone() override { return value >= TARGET_MOSAIC; }

 private:
  u32 value = 0;
};
