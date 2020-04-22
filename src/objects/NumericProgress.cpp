#include "NumericProgress.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 DIGIT_POSITION_X = 79;
const u32 DIGIT_POSITION_Y = 132;

NumericProgress::NumericProgress() {
  for (u32 i = 0; i < 3; i++) {
    auto digit = std::unique_ptr<ComboDigit>{new ComboDigit(i)};
    digit->get()->moveTo(DIGIT_POSITION_X + i * DIGIT_WIDTH, DIGIT_POSITION_Y);
    digits.push_back(std::move(digit));
  }
}

void NumericProgress::setValue(u32 current, u32 total) {
  digits[0]->set(Div(current, 100), false);
  digits[1]->set(Div(DivMod(current, 100), 10), false);
  digits[2]->set(DivMod(current, 10), false);
}

void NumericProgress::render(std::vector<Sprite*>* sprites) {
  for (auto& digit : digits)
    sprites->push_back(digit->get());
}

NumericProgress::~NumericProgress() {
  digits.clear();
}
