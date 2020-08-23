#ifndef COMBO_H
#define COMBO_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "ComboTitle.h"
#include "objects/Digit.h"

class Combo {
 public:
  Combo();

  void setValue(int value);
  inline u32 getValue() { return value; }
  void show();
  void hide();
  void relocate();

  void tick();
  void render(std::vector<Sprite*>* sprites);

  ~Combo();

 private:
  u32 value = 0;
  std::unique_ptr<ComboTitle> title;
  std::vector<std::unique_ptr<Digit>> digits;
};

#endif  // COMBO_H
