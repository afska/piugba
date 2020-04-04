#ifndef COMBO_TITLE_H
#define COMBO_TITLE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/AnimatedIndicator.h"

class ComboTitle : public AnimatedIndicator {
 public:
  ComboTitle();

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // COMBO_TITLE_H
