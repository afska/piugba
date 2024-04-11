#ifndef LIFE_BAR_H
#define LIFE_BAR_H

#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite.h>

const int INITIAL_LIFE = 60;
const int MAX_LIFE = 100;
const int MIN_LIFE = -13;
const u32 LIFEBAR_POSITION_X = 15;
const int LIFEBAR_POSITION_Y = -20 + 2;

class LifeBar {
 public:
  LifeBar(u8 playerId);

  inline u8 getMosaicValue() { return mosaicValue; }

  void setLife(int life);
  void blink();
  void die();
  void relocate();

  inline bool getIsDead() { return isDead; }

  void tick(ForegroundPaletteManager* foregroundPalette);
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u8 playerId;
  u32 value;
  u32 absLife = INITIAL_LIFE;
  int animatedOffset = 0;
  u32 animatedRainbowOffset = 0;
  u32 wait = 0;
  u32 blinkWait = 0;
  u8 mosaicValue = 0;
  bool animatedFlag = false;
  bool isDead = false;
  bool isModern;

  void paint(ForegroundPaletteManager* foregroundPalette);
};

// TODO: RESTORE CLASSIC LIFEBAR LUT
const u8 LIFE_TO_VALUE_LUT[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  2,  2,
    2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,
    4,  4,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  7,
    7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,
    9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11,
    11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14};

const u8 LIFE_TO_MOSAIC_LUT[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#endif  // LIFE_BAR_H
