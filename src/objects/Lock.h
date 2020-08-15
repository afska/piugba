#ifndef LOCK_H
#define LOCK_H

#include <libgba-sprite-engine/sprites/sprite.h>

class Lock {
 public:
  Lock(u32 x, u32 y, bool reuseTiles);

  bool isVisible();
  void setVisible(bool isVisible);

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;

  u32 x;
  u32 y;
};

#endif  // LOCK_H
