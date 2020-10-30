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
  inline bool hasBeenReleasedNow() { return isNewReleaseEvent; }

  inline bool getHandledFlag() { return handledFlag; }
  inline void setHandledFlag(bool value) { handledFlag = value; }

  inline void setIsPressed(bool isPressed) {
    bool isNewPressEvent = !this->isPressed && isPressed;
    bool isNewReleaseEvent = this->isPressed && !isPressed;
    this->isPressed = isPressed;

    this->isNewPressEvent = isNewPressEvent;
    this->isNewReleaseEvent = isNewReleaseEvent;
    IFSTRESSTEST { this->isNewPressEvent = isPressed; }
  }

 protected:
  bool isPressed = false;
  bool isNewPressEvent = false;
  bool isNewReleaseEvent;
  bool handledFlag = false;
};

#endif  // INPUT_HANDLER_H
