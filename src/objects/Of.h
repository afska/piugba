#ifndef OF_H
#define OF_H

#include <libgba-sprite-engine/sprites/sprite.h>

class Of {
 public:
  Of(u32 x, u32 y);

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // OF_H
