#include "Total.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 MAX_TOTAL = 9999;
const u32 DIGITS = 4;

Total::Total(u32 x, u32 y, bool isFirst) {
  for (u32 i = 0; i < DIGITS; i++) {
    auto digit =
        std::unique_ptr<Digit>{new Digit(DigitSize::MINI, x, y, i, false)};
    digit->showAt(0);

    if (i == 0 && !isFirst)
      SPRITE_reuseTiles(digit->get());

    digits.push_back(std::move(digit));
  }
}

void Total::setValue(u32 value) {
  if (value > MAX_TOTAL)
    value = MAX_TOTAL;

  for (int i = DIGITS - 1; i >= 0; i--) {
    digits[i]->set(value % 10, false);
    value /= 10;
  }
}

void Total::render(std::vector<Sprite*>* sprites) {
  for (auto& it : digits)
    sprites->push_back(it->get());
}

Total::~Total() {
  digits.clear();
}
