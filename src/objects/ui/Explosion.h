#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <libgba-sprite-engine/sprites/sprite.h>

class Explosion {
 public:
  Explosion(u32 x, u32 y, bool reuseTiles);

  bool isVisible();
  void setVisible(bool isVisible);

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;

  u32 x;
  u32 y;
  u32 animationFrame;
};

#endif  // EXPLOSION_H
