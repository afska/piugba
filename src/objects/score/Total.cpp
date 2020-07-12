#include "Total.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 MAX_TOTAL = 999;
const u32 DIGITS = 3;
const u32 DIGIT_POSITION_X = 8;

Total::Total(u32 y) {
  for (u32 i = 0; i < 3; i++) {
    auto digit = std::unique_ptr<Digit>{
        new Digit(DigitSize::BIG, DIGIT_POSITION_X, y, i)};
    digit->showAt(0);
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
  for (auto& digit : digits)
    sprites->push_back(digit->get());
}

Total::~Total() {
  digits.clear();
}
