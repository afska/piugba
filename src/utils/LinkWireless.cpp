#include "LinkWireless.hpp"

#ifdef LINK_WIRELESS_PUT_ISR_IN_IWRAM
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onSerial() {
  REG_IME = 1;  // [!]
  __onSerial();
}
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onTimer() {
  REG_IME = 1;  // [!]
  __onTimer();
}
LINK_WIRELESS_CODE_IWRAM void LinkWireless::_onACKTimer() {
  REG_IME = 1;  // [!]
  __onACKTimer();
}
#endif
