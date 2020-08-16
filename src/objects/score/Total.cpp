#include "Total.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 MAX_TOTAL = 999;
const u32 DIGITS = 3;
const u32 DIGITS_POSITION_X = 160;

Total::Total(u32 y, bool isFirst) {
  for (u32 i = 0; i < DIGITS; i++) {
    auto digit = std::unique_ptr<Digit>{
        new Digit(DigitSize::MINI, DIGITS_POSITION_X, y, i)};
    digit->showAt(0);

    if (i == 0 && !isFirst)
      SPRITE_reuseTiles(digit->get());

    digits.push_back(std::move(digit));
  }
}

void Total::setValue(u32 value) {
  if (value > MAX_TOTAL)
    value = MAX_TOTAL;

  digits[0]->set(THREE_DIGITS_LUT[value * DIGITS], false);
  digits[1]->set(THREE_DIGITS_LUT[value * DIGITS + 1], false);
  digits[2]->set(THREE_DIGITS_LUT[value * DIGITS + 2], false);
}

void Total::render(std::vector<Sprite*>* sprites) {
  for (auto& it : digits)
    sprites->push_back(it->get());
}

Total::~Total() {
  digits.clear();
}
