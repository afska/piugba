#ifndef COMBO_TITLE_H
#define COMBO_TITLE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/base/AnimatedIndicator.h"

class ComboTitle : public AnimatedIndicator {
 public:
  ComboTitle(u8 playerId);

  void relocate();

  Sprite* get() override;

 private:
  std::unique_ptr<Sprite> sprite;
  u8 playerId;
};

#endif  // COMBO_TITLE_H
