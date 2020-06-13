#include "Combo.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "utils/SpriteUtils.h"

u32 MAX_COMBO = 999;
u32 DIGITS = 3;

Combo::Combo() {
  title = std::unique_ptr<ComboTitle>{new ComboTitle()};

  for (u32 i = 0; i < 3; i++) {
    auto digit = std::unique_ptr<ComboDigit>{new ComboDigit(i)};
    digits.push_back(std::move(digit));
  }
}

void Combo::setValue(int value) {
  bool isRed = value < 0;
  u32 absValue = min(abs(value), MAX_COMBO);

  this->value = absValue;
  digits[0]->set(COMBO_VALUE_LUT[absValue * DIGITS], isRed);
  digits[1]->set(COMBO_VALUE_LUT[absValue * DIGITS + 1], isRed);
  digits[2]->set(COMBO_VALUE_LUT[absValue * DIGITS + 2], isRed);

  // Without optimizations:
  // digits[0]->set(Div(absValue, 100), isRed);
  // digits[1]->set(Div(DivMod(absValue, 100), 10), isRed);
  // digits[2]->set(DivMod(absValue, 10), isRed);
}

void Combo::show() {
  title->show();

  for (auto& digit : digits)
    digit->show();
}

void Combo::hide() {
  SPRITE_hide(title->get());

  for (auto& digit : digits)
    SPRITE_hide(digit->get());
}

void Combo::tick() {
  title->tick();

  for (auto& digit : digits)
    digit->tick();
}

void Combo::render(std::vector<Sprite*>* sprites) {
  sprites->push_back(title->get());

  for (auto& digit : digits)
    sprites->push_back(digit->get());
}

Combo::~Combo() {
  digits.clear();
}
