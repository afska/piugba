#include "LifeBar.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

#include "data/content/_compiled_sprites/spr_lifebar.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/ArrowInfo.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

#define LIFEBAR_COLORS 18

const u32 ANIMATION_OFFSET = 2;
const u32 WAIT_TIME = 3;
const u32 MIN_VALUE = 0;
const u32 ALMOST_MIN_VALUE = 1;
const u32 MAX_VALUE = 10;
const u32 MIN_ANIMATED_VALUE = 1;
const u32 UNIT = 2;
const u16 PALETTE_COLORS[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {127, 4345, 410, 7606, 2686, 1595, 766, 700, 927, 894, 988, 923, 1017, 951,
     974, 879, 9199, 936},
    {126, 4344, 409, 7605, 1629, 1562, 733, 667, 862, 829, 922, 890, 984, 918,
     942, 846, 8142, 904}};
const u8 PALETTE_INDEXES[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {202, 205, 211, 212, 223, 219, 232, 225, 248, 241, 249, 243, 250, 242, 238,
     231, 245, 230},
    {201, 204, 209, 210, 221, 216, 229, 222, 240, 235, 246, 237, 247, 236, 233,
     228, 239, 227}};
const COLOR DISABLED_COLOR = 0x0000;
const COLOR DISABLED_COLOR_BORDER = 0x2529;
const COLOR CURSOR_COLOR = 0x7FD8;
const COLOR CURSOR_COLOR_BORDER = 0x7734;
const COLOR BLINK_MIN_COLOR = 127;
const COLOR BLINK_MAX_COLOR = 0x7FFF;

LifeBar::LifeBar(u8 playerId) {
  SpriteBuilder<Sprite> builder;
  sprite = builder.withData(spr_lifebarTiles, sizeof(spr_lifebarTiles))
               .withSize(SIZE_64_32)
               .withLocation((isCoop() ? GAME_POSITION_X[1]
                                       : GameState.positionX[playerId]) +
                                 LIFEBAR_POSITION_X,
                             GameState.positionY + LIFEBAR_POSITION_Y)
               .buildPtr();

  if (playerId > 0)
    SPRITE_reuseTiles(sprite.get());

  this->playerId = playerId;
}

void LifeBar::setLife(int life) {
  u32 absLife = max(life, 0);
  value = LIFE_TO_VALUE_LUT[absLife];
  mosaicValue = LIFE_TO_MOSAIC_LUT[absLife];
}

void LifeBar::blink(ForegroundPaletteManager* foregroundPalette) {
  animatedValue = value;
  wait = WAIT_TIME;
}

void LifeBar::die() {
  isDead = true;
}

void LifeBar::tick(ForegroundPaletteManager* foregroundPalette) {
  paint(foregroundPalette);

  if (playerId > 0)
    sprite->flipVertically(true);

  if (wait == 0 || wait == 2)
    animatedFlag = !animatedFlag;

  if (animatedValue > (u32)max(MIN_ANIMATED_VALUE, value - ANIMATION_OFFSET) &&
      wait == 0) {
    animatedValue--;
    wait = WAIT_TIME;
  } else if (wait > 0)
    wait--;
}

void LifeBar::paint(ForegroundPaletteManager* foregroundPalette) {
  bool isBorder = false;

  for (u32 i = 0; i < LIFEBAR_COLORS; i++) {
    COLOR color;

    if (isDead)
      color = isBorder ? DISABLED_COLOR_BORDER : BLINK_MIN_COLOR;
    else if (value <= ALMOST_MIN_VALUE) {
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

      color = PALETTE_COLORS[playerId][i];
      if (i >= animatedIndex * UNIT)
        color = disabled;
      if (i >= index * UNIT && i <= index * UNIT + 1)
        color = cursor;
    } else {
      // blink green
      color = (animatedFlag && !isBorder) ? BLINK_MAX_COLOR
                                          : PALETTE_COLORS[playerId][i];
    }

    foregroundPalette->change(0, PALETTE_INDEXES[playerId][i], color);
    isBorder = !isBorder;
  }
}
