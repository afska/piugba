#include "Combo.h"
#include <libgba-sprite-engine/gba/tonc_bios.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>

Combo::Combo() {
  title = std::unique_ptr<ComboTitle>{new ComboTitle()};

  for (u32 i = 0; i < 3; i++) {
    auto digit = std::unique_ptr<ComboDigit>{new ComboDigit(i)};
    digits.push_back(std::move(digit));
  }
}

void Combo::setValue(u32 value) {
  this->value = value;

  digits[0]->set(Div(value, 100));
  digits[1]->set(Div(DivMod(value, 100), 10));
  digits[2]->set(DivMod(value, 10));
}

u32 Combo::getValue() {
  return value;
}

void Combo::show() {
  title->show();

  for (auto& digit : digits)
    digit->show();
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
