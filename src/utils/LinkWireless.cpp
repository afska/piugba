#include "LinkWireless.hpp"

#ifdef LINK_WIRELESS_PUT_ISR_IN_IWRAM
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onSerial() {
  // [!]
  if (!isHandlingInterrupt)
    isHandlingInterrupt = true;
  else
    return;
  REG_IME = 1;
  __onSerial();
  isHandlingInterrupt = false;
}
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onTimer() {
  // [!]
  if (!isHandlingInterrupt)
    isHandlingInterrupt = true;
  else
    return;
  REG_IME = 1;
  __onTimer();
  isHandlingInterrupt = false;
}
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onACKTimer() {
  // [!]
  if (!isHandlingInterrupt)
    isHandlingInterrupt = true;
  else
    return;
  REG_IME = 1;
  __onACKTimer();
  isHandlingInterrupt = false;
}
#endif
