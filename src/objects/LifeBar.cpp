#include "LifeBar.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/compiled/spr_lifebar.h"

const u32 POSITION_X = 15;
const int POSITION_Y = -11 + 2;
const u32 ANIMATION_OFFSET = 2;
const u32 WAIT_TIME = 3;
const u32 MIN_VALUE = 0;
const u32 ALMOST_MIN_VALUE = 1;
const u32 MAX_VALUE = 10;
const u32 MIN_ANIMATED_VALUE = 1;
const u32 UNIT = 2;
const u16 PALETTE_COLORS[] = {127, 4345, 410, 7606, 2686, 1595, 766, 700,  927,
                              894, 988,  923, 1017, 951,  974,  879, 9199, 936};
const u8 PALETTE_INDEXES[] = {173, 175, 179, 180, 188, 186, 194, 190, 202,
                              197, 203, 199, 204, 198, 196, 193, 201, 192};
const COLOR DISABLED_COLOR = 0x0000;
const COLOR DISABLED_COLOR_BORDER = 0x2529;
const COLOR CURSOR_COLOR = 0x7FD8;
const COLOR CURSOR_COLOR_BORDER = 0x7734;
const COLOR BLINK_MIN_COLOR = 127;
const COLOR BLINK_MAX_COLOR = 0x7FFF;

LifeBar::LifeBar() {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_lifebarTiles, sizeof(spr_lifebarTiles))
               .withSize(SIZE_64_32)
               .withLocation(POSITION_X, POSITION_Y)
               .buildPtr();
}

void LifeBar::setLife(int life) {
  value = Div(life < 0 ? 0 : life, 10);
}

void LifeBar::blink(ForegroundPaletteManager* foregroundPalette) {
  animatedValue = value;
  wait = WAIT_TIME;
}

void LifeBar::tick(ForegroundPaletteManager* foregroundPalette) {
  paint(foregroundPalette);

  if (wait == 0 || wait == 2)
    animatedFlag = !animatedFlag;

  if (animatedValue > (u32)max(MIN_ANIMATED_VALUE, value - ANIMATION_OFFSET) &&
      wait == 0) {
    animatedValue--;
    wait = WAIT_TIME;
  } else if (wait > 0)
    wait--;
}

Sprite* LifeBar::get() {
  return sprite.get();
}

void LifeBar::paint(ForegroundPaletteManager* foregroundPalette) {
  bool isBorder = false;

  for (u32 i = 0; i < sizeof(PALETTE_INDEXES); i++) {
    COLOR color;

    if (value <= ALMOST_MIN_VALUE) {
      COLOR redBlink = animatedFlag ? BLINK_MIN_COLOR : DISABLED_COLOR;
      COLOR disabled = isBorder ? DISABLED_COLOR_BORDER : DISABLED_COLOR;

      if (value == MIN_VALUE)
        // red blink
        color = isBorder ? DISABLED_COLOR_BORDER : redBlink;
      else
        // red mini blink
        color =
            i < UNIT ? (isBorder ? DISABLED_COLOR_BORDER : redBlink) : disabled;
    } else if (value < MAX_VALUE) {
      // middle
      COLOR disabled = isBorder ? DISABLED_COLOR_BORDER : DISABLED_COLOR;
      COLOR cursor = isBorder ? CURSOR_COLOR_BORDER : CURSOR_COLOR;
      u32 index = value - 1;
      u32 animatedIndex = animatedValue - 1;

      color = PALETTE_COLORS[i];
      if (i >= animatedIndex * UNIT)
        color = disabled;
      if (i >= index * UNIT && i <= index * UNIT + 1)
        color = cursor;
    } else {
      // blink green
      color = (animatedFlag && !isBorder) ? BLINK_MAX_COLOR : PALETTE_COLORS[i];
    }

    foregroundPalette->change(0, PALETTE_INDEXES[i], color);
    isBorder = !isBorder;
  }
}
