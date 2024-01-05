#include "NumericProgress.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 DIGITS = 2;
const u32 OF_POSITION_X = 41;
const u32 DIGITS_TOTAL_POSITION_X = 58;

NumericProgress::NumericProgress(u32 x, u32 y) {
  for (u32 i = 0; i < DIGITS; i++) {
    auto digit =
        std::unique_ptr<Digit>{new Digit(DigitSize::BIG, x, y, i, false)};
    digit->showAt(0);
    completedDigits.push_back(std::move(digit));
  }

  for (u32 i = 0; i < DIGITS; i++) {
    auto digit = std::unique_ptr<Digit>{
        new Digit(DigitSize::BIG, x + DIGITS_TOTAL_POSITION_X, y, i, false)};
    SPRITE_reuseTiles(digit->get());
    digit->showAt(0);
    totalDigits.push_back(std::move(digit));
  }

  of = std::unique_ptr<Of>{new Of(x + OF_POSITION_X, y)};
}

void NumericProgress::setValue(u32 completed, u32 total) {
  completedDigits[0]->set(THREE_DIGITS_LUT[completed * LUT_DIGITS + 1], false);
  completedDigits[1]->set(THREE_DIGITS_LUT[completed * LUT_DIGITS + 2], false);

  totalDigits[0]->set(THREE_DIGITS_LUT[total * LUT_DIGITS + 1], false);
  totalDigits[1]->set(THREE_DIGITS_LUT[total * LUT_DIGITS + 2], false);
}

void NumericProgress::render(std::vector<Sprite*>* sprites) {
  for (auto& it : completedDigits)
    sprites->push_back(it->get());

  for (auto& it : totalDigits)
    sprites->push_back(it->get());

  sprites->push_back(of->get());
}

NumericProgress::~NumericProgress() {
  completedDigits.clear();
  totalDigits.clear();
}
