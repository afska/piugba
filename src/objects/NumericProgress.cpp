#include "NumericProgress.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include "utils/SpriteUtils.h"

const u32 DIGIT_CURRENT_POSITION_X = 63;
const u32 DIGIT_TOTAL_POSITION_X = 121;
const u32 OF_POSITION_X = 104;
const u32 DIGIT_POSITION_Y = 131;
const u32 OF_POSITION_Y = DIGIT_POSITION_Y;

NumericProgress::NumericProgress() {
  for (u32 i = 0; i < 2; i++) {
    auto digit = std::unique_ptr<ComboDigit>{new ComboDigit(i)};
    digit->get()->moveTo(DIGIT_CURRENT_POSITION_X + i * DIGIT_WIDTH,
                         DIGIT_POSITION_Y);
    completedDigits.push_back(std::move(digit));
  }

  for (u32 i = 3; i < 5; i++) {
    auto digit = std::unique_ptr<ComboDigit>{new ComboDigit(i)};
    digit->get()->moveTo(DIGIT_TOTAL_POSITION_X + (i - 3) * DIGIT_WIDTH,
                         DIGIT_POSITION_Y);
    totalDigits.push_back(std::move(digit));
  }

  of = std::unique_ptr<Of>{new Of(OF_POSITION_X, OF_POSITION_Y)};
}

void NumericProgress::setValue(u32 completed, u32 total) {
  completedDigits[0]->set(Div(completed, 10), false);
  completedDigits[1]->set(Div(DivMod(completed, 10), 10), false);

  totalDigits[0]->set(Div(total, 10), false);
  totalDigits[1]->set(Div(DivMod(total, 10), 10), false);
}

void NumericProgress::render(std::vector<Sprite*>* sprites) {
  for (auto& digit : completedDigits)
    sprites->push_back(digit->get());

  for (auto& digit : totalDigits)
    sprites->push_back(digit->get());

  sprites->push_back(of->get());
}

NumericProgress::~NumericProgress() {
  completedDigits.clear();
  totalDigits.clear();
}
