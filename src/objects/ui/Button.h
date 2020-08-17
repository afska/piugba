#ifndef BUTTON_H
#define BUTTON_H

#include <libgba-sprite-engine/sprites/sprite.h>

enum ButtonType { BLUE, GREEN, ORANGE, LEVEL_METER };

const u32 BUTTONS_TOTAL = 3;

class Button {
 public:
  Button(ButtonType, u32 x, u32 y, bool reuseTiles);

  void setSelected(bool isSelected);

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  ButtonType type;
  bool isSelected = false;
};

#endif  // BUTTON_H
