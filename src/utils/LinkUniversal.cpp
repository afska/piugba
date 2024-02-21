#include "LinkUniversal.hpp"

#pragma GCC push_options
#pragma GCC optimize("Os")

LINK_WIRELESS_CODE_IWRAM void LINK_UNIVERSAL_ISR_SERIAL() {
  linkUniversal->_onSerial();
}

LINK_WIRELESS_CODE_IWRAM void LINK_UNIVERSAL_ISR_TIMER() {
  linkUniversal->_onTimer();
}

LINK_WIRELESS_CODE_IWRAM void LINK_UNIVERSAL_ISR_ACK_TIMER() {
  linkUniversal->_onACKTimer();
}

#pragma GCC pop_options
