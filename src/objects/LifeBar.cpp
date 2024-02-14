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

const int ANIMATION_OFFSET = 2;
const u32 WAIT_TIME = 3;
const u32 MIN_VALUE = 0;
const u32 ALMOST_MIN_VALUE = 1;
const u32 MAX_VALUE = 10;
const u32 UNIT = 2;
const u16 PALETTE_COLORS[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {127, 4345, 410, 7606, 2686, 1595, 766, 700, 927, 894, 988, 923, 1017, 951,
     974, 879, 9199, 936},
    {126, 4344, 409, 7605, 1629, 1562, 733, 667, 862, 829, 922, 890, 984, 918,
     942, 846, 8142, 904}};
const u8 PALETTE_INDEXES[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {198, 201, 207, 208, 219, 215, 228, 221, 244, 237, 245, 239, 246, 238, 234,
     227, 241, 226},
    {197, 200, 205, 206, 217, 212, 225, 218, 236, 231, 242, 233, 243, 232, 229,
     224, 235, 223}};
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
               .withLocation((isDouble() ? GAME_POSITION_X[1]
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

void LifeBar::blink() {
  animatedOffset = 0;
  wait = WAIT_TIME;
}

void LifeBar::die() {
  isDead = true;
}

void LifeBar::relocate() {
  sprite->moveTo(
      (isDouble() ? GAME_POSITION_X[1] : GameState.positionX[playerId]) +
          LIFEBAR_POSITION_X,
      sprite->getY());
}

void LifeBar::tick(ForegroundPaletteManager* foregroundPalette) {
  paint(foregroundPalette);

  if (playerId > 0)
    sprite->flipVertically(true);

  blinkWait++;
  if (blinkWait == 2) {
    blinkWait = 0;
    animatedFlag = !animatedFlag;
  }

  if (animatedOffset > -ANIMATION_OFFSET && wait == 0) {
    animatedOffset--;
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
      u32 animatedIndex = max(index + animatedOffset, 0);

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
    if (GameState.mods.colorFilter != ColorFilter::NO_FILTER)
      SCENE_applyColorFilterIndex(foregroundPalette, 0,
                                  PALETTE_INDEXES[playerId][i],
                                  GameState.mods.colorFilter);

    isBorder = !isBorder;
  }
}
