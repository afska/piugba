#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <libgba-sprite-engine/gba_engine.h>

class InputHandler {
 public:
  bool getIsPressed();
  bool hasBeenPressedNow();
  void setIsPressed(bool isPressed);

 protected:
  bool isPressed = false;
  bool isNewPressEvent = false;
};

#endif  // INPUT_HANDLER_H
