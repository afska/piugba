#include "Combo.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "gameplay/multiplayer/Syncer.h"
#include "objects/ArrowInfo.h"
#include "utils/SpriteUtils.h"

const u32 MAX_COMBO = 999;
const u32 DIGITS = 3;
const u32 DIGITS_POSITION_X = 8;
const u32 DIGITS_POSITION_Y = 89;

Combo::Combo(u8 playerId) {
  this->playerId = playerId;

  title = std::unique_ptr<ComboTitle>{new ComboTitle(playerId)};

  for (u32 i = 0; i < DIGITS; i++) {
    auto digit = std::unique_ptr<Digit>{new Digit(
        DigitSize::BIG,
        (isCoop() ? GAME_POSITION_X[1] : GameState.positionX[playerId]) +
            DIGITS_POSITION_X,
        GameState.scorePositionY + DIGITS_POSITION_Y, i, playerId > 0)};
    digits.push_back(std::move(digit));
  }
}

void Combo::setValue(int value) {
  bool isRed = value < 0;
  u32 absValue = min(abs(value), MAX_COMBO);

  this->value = absValue;
  digits[0]->set(THREE_DIGITS_LUT[absValue * LUT_DIGITS], isRed);
  digits[1]->set(THREE_DIGITS_LUT[absValue * LUT_DIGITS + 1], isRed);
  digits[2]->set(THREE_DIGITS_LUT[absValue * LUT_DIGITS + 2], isRed);

  // Without optimizations:
  // digits[0]->set(Div(absValue, 100), isRed);
  // digits[1]->set(Div(DivMod(absValue, 100), 10), isRed);
  // digits[2]->set(DivMod(absValue, 10), isRed);
}

void Combo::show() {
  title->show();

  for (auto& it : digits)
    it->show();
}

void Combo::hide() {
  SPRITE_hide(title->get());

  for (auto& it : digits)
    SPRITE_hide(it->get());
}

void Combo::relocate() {
  title->relocate();

  for (u32 i = 0; i < DIGITS; i++)
    digits[i]->relocate(
        DigitSize::BIG,
        (isCoop() ? GAME_POSITION_X[1] : GameState.positionX[playerId]) +
            DIGITS_POSITION_X,
        GameState.scorePositionY + DIGITS_POSITION_Y, i);
}

void Combo::tick() {
  title->tick();

  for (auto& it : digits)
    it->tick();
}

Combo::~Combo() {
  digits.clear();
}
