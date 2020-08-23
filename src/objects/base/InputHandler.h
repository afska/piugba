#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <libgba-sprite-engine/gba_engine.h>

#include "objects/ArrowInfo.h"

class InputHandler {
 public:
  InputHandler() {
    this->isPressed = true;
    // it starts as `true` to avoid firing events if the
    // key is already pressed when the scene starts
  }

  inline bool getIsPressed() { return isPressed; }

  inline bool hasBeenPressedNow() { return isNewPressEvent; }

  inline void setIsPressed(bool isPressed) {
    bool isNewPressEvent = !this->isPressed && isPressed;
    this->isPressed = isPressed;

    this->isNewPressEvent = isNewPressEvent;
    IFSTRESSTEST { this->isNewPressEvent = isPressed; }
  }

 protected:
  bool isPressed = false;
  bool isNewPressEvent = false;
};

#endif  // INPUT_HANDLER_H
