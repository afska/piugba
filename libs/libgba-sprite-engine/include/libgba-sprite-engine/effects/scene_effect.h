#ifndef GBA_SPRITE_ENGINE_SCENE_EFFECT_H
#define GBA_SPRITE_ENGINE_SCENE_EFFECT_H

#pragma GCC system_header

#include <libgba-sprite-engine/scene.h>

class SceneEffect {
 public:
  void setSceneToAffect(Scene* scene) { sceneToAffect = scene; };

  virtual void render() = 0;
  virtual bool isDone() = 0;

 protected:
  // WHY no reference? Scene& operator= is implicitly deleted and no intentions
  // to use that
  Scene* sceneToAffect;
};

#endif  // GBA_SPRITE_ENGINE_SCENE_EFFECT_H
