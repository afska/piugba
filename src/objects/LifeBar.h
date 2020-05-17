#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite.h>

const int INITIAL_LIFE = 60;
const int MAX_LIFE = 100;
const int MIN_LIFE = -13;

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

const u8 LIFE_TO_VALUE_LUT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10};

#endif  // LIFE_BAR_H
