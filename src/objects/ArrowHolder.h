#ifndef ARROW_HOLDER_H
#define ARROW_HOLDER_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include "Arrow.h"

class ArrowHolder {
 public:
  ArrowHolder(ArrowType type);

  void update();
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  ArrowType type;
  bool flip = false;
};

#endif  // ARROW_HOLDER_H
