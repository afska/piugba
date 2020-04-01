#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite.h>

class LifeBar {
 public:
  LifeBar();

  void blink(ForegroundPaletteManager* foregroundPalette);

  void tick(ForegroundPaletteManager* foregroundPalette);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 value = 6;
  u32 animatedValue = value;
  u32 wait = 0;
  bool animatedFlag = false;

  void paint(ForegroundPaletteManager* foregroundPalette);
};

#endif  // LIFE_BAR_H
