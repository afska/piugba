#include "LifeBar.h"

#include <libgba-sprite-engine/gba/tonc_math.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

// #include "data/content/_compiled_sprites/spr_lifebar.h"
// TODO: CLASSIC LIFE BAR
#include "data/content/_compiled_sprites/spr_lifebar_mdrn.h"
#include "gameplay/multiplayer/Syncer.h"
#include "objects/ArrowInfo.h"
#include "utils/EffectUtils.h"
#include "utils/SpriteUtils.h"

#define THEME_COUNT 2
#define LIFEBAR_COLORS 27  // TODO: OR 18 ON CLASSIC LIFEBAR

const int ANIMATION_OFFSET = 2;
const u32 WAIT_TIME = 3;
const u32 MIN_VALUE = 0;
const u32 ALMOST_MIN_VALUE = 2;  // TODO: OR 1 ON CLASSIC LIFEBAR
const u32 MAX_VALUE = 14;        // TODO: OR 10 ON CLASSIC LIFEBAR
const u32 UNIT = 2;
// TODO: RESTORE CLASSIC LIFE BAR
const u16 PALETTE_COLORS[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {63,    350,   542,   1662,  734,   861,   927,   989,   890,
     918,   850,   809,   960,   9152,  19392, 21312, 25442, 27456,
     26272, 26208, 32384, 31264, 30048, 30912, 30724, 27661, 26644},
    {61,    348,   508,   604,   700,   795,   797,   828,   824,
     853,   785,   744,   896,   9088,  18304, 22368, 23330, 26368,
     24160, 24096, 30304, 29184, 28000, 28864, 28676, 25612, 24595}};
const u8 PALETTE_INDEXES[GAME_MAX_PLAYERS][LIFEBAR_COLORS] = {
    {194, 201, 212, 222, 232, 242, 245, 246, 243, 241, 234, 219, 230, 231,
     233, 218, 228, 221, 207, 203, 206, 202, 197, 192, 186, 188, 191},
    {193, 198, 208, 215, 225, 236, 237, 240, 238, 235, 229, 213, 220, 223,
     226, 224, 217, 214, 205, 200, 204, 199, 196, 190, 185, 187, 189}};
const COLOR DISABLED_COLOR = 0x0000;
const COLOR DISABLED_COLOR_BORDER = 0x2529;
const COLOR CURSOR_COLOR = 0x7FD8;
const COLOR CURSOR_COLOR_BORDER = 0x7734;
const COLOR BLINK_MIN_COLOR = 127;
const COLOR BLINK_MAX_COLOR = 0x7FFF;

LifeBar::LifeBar(u8 playerId) {
  SpriteBuilder<Sprite> builder;
  sprite =
      builder  // TODO: CLASSIC LIFE BAR
          .withData(SAVEFILE_isUsingModernTheme()
                        ? spr_lifebar_mdrnTiles
                        : spr_lifebar_mdrnTiles /*spr_lifebarTiles*/,
                    SAVEFILE_isUsingModernTheme()
                        ? sizeof(spr_lifebar_mdrnTiles)
                        : sizeof(spr_lifebar_mdrnTiles /*spr_lifebarTiles*/))
          .withSize(SIZE_64_32)
          .withLocation((isDouble() ? GAME_POSITION_X[1]
                                    : GameState.positionX[playerId]) +
                            LIFEBAR_POSITION_X,
                        GameState.positionY + LIFEBAR_POSITION_Y)
          .buildPtr();

  if (playerId > 0)
    SPRITE_reuseTiles(sprite.get());

  this->playerId = playerId;
  sprite->flipVertically(playerId > 0);
  isModern = SAVEFILE_isUsingModernTheme();
  value = INITIAL_LIFE * MAX_VALUE / MAX_LIFE;
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

  if (blinkWait == 0)
    animatedRainbowOffset++;
  if (animatedRainbowOffset >= LIFEBAR_COLORS / 2)
    animatedRainbowOffset = 0;
}

CODE_IWRAM void LifeBar::paint(ForegroundPaletteManager* foregroundPalette) {
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
      if (isModern) {
        // rainbow
        color = PALETTE_COLORS[playerId][(
            (LIFEBAR_COLORS - 1 - i + animatedRainbowOffset * 2) %
            LIFEBAR_COLORS)];
      } else {
        // blink green
        color = (animatedFlag && !isBorder) ? BLINK_MAX_COLOR
                                            : PALETTE_COLORS[playerId][i];
      }
    }

    foregroundPalette->change(0, PALETTE_INDEXES[playerId][i], color);
    if (GameState.mods.colorFilter != ColorFilter::NO_FILTER)
      SCENE_applyColorFilterIndex(foregroundPalette, 0,
                                  PALETTE_INDEXES[playerId][i],
                                  GameState.mods.colorFilter);

    isBorder = !isBorder;
  }
}
