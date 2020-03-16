#ifndef COMBO_H
#define COMBO_H

#include <libgba-sprite-engine/sprites/sprite.h>
#include <vector>
#include "ComboDigit.h"
#include "ComboTitle.h"

class Combo {
 public:
  Combo();

  void setValue(u32 value);
  u32 getValue();
  void show();
  void hide();

  void tick();
  void render(std::vector<Sprite*>* sprites);

 private:
  u32 value = 0;
  std::unique_ptr<ComboTitle> title;
  std::vector<std::unique_ptr<ComboDigit>> digits;
};

#endif  // COMBO_H
