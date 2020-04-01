#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite.h>

class LifeBar {
 public:
  LifeBar();

  void tick(ForegroundPaletteManager* foregroundPalette);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
};

#endif  // LIFE_BAR_H
