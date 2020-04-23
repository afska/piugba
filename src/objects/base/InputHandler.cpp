#include "InputHandler.h"

bool InputHandler::getIsPressed() {
  return isPressed;
}

bool InputHandler::hasBeenPressedNow() {
  return isNewPressEvent;
}

void InputHandler::setIsPressed(bool isPressed) {
  bool isNewPressEvent = !this->isPressed && isPressed;
  this->isPressed = isPressed;

  this->isNewPressEvent = isNewPressEvent;
}