#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite.h>

const int INITIAL_LIFE = 60;
const int MAX_LIFE = 100;
const int MIN_LIFE = -25;

class LifeBar {
 public:
  LifeBar();

  void setLife(int life);
  void blink(ForegroundPaletteManager* foregroundPalette);

  void tick(ForegroundPaletteManager* foregroundPalette);
  Sprite* get();

 private:
  std::unique_ptr<Sprite> sprite;
  u32 value = Div(INITIAL_LIFE, 10);
  u32 animatedValue = value;
  u32 wait = 0;
  bool animatedFlag = false;

  void paint(ForegroundPaletteManager* foregroundPalette);
};

#endif  // LIFE_BAR_H
