#ifndef GBA_SPRITE_ENGINE_GBAENGINE_H
#define GBA_SPRITE_ENGINE_GBAENGINE_H

#pragma GCC system_header

#include <libgba-sprite-engine/effects/scene_effect.h>
#include <libgba-sprite-engine/gba/tonc_memdef.h>
#include <libgba-sprite-engine/gba/tonc_memmap.h>
#include <libgba-sprite-engine/sprites/sprite_manager.h>

#include "scene.h"

#define GBA_SCREEN_WIDTH 240
#define GBA_SCREEN_HEIGHT 160

class GBAEngine {
 public:
  GBAEngine();

  void setScene(Scene* scene);
  void transitionIntoScene(Scene* scene, SceneEffect* effect);
  bool isTransitioning() { return currentEffectForTransition != nullptr; }
  void disableText() { this->disableTextBg = true; }
  void enableText() { this->disableTextBg = false; }

  inline void update() {
    if (sceneToTransitionTo && currentEffectForTransition->isDone()) {
      setScene(sceneToTransitionTo);
    }

    u16 keys = ~REG_KEYS & KEY_ANY;
    currentScene->tick(keys);

    // (call render() on vsync)
  }

  inline void render() {
    if (sceneToTransitionTo) {
      currentEffectForTransition->render();
    }

    currentScene->render();
    spriteManager.render();
  }

  void updateSpritesInScene();

 private:
  // WHY raw pointers? the engine does the transition and cleanup work itself
  Scene* currentScene;
  Scene* sceneToTransitionTo;
  SceneEffect* currentEffectForTransition;

  bool disableTextBg;
  SpriteManager spriteManager;

  void cleanupPreviousScene();
};

#endif  // GBA_SPRITE_ENGINE_GBAENGINE_H
