#include "LinkWireless.hpp"

// [!]
// This library has some tweaks (marked with "[!]") for piuGBA.
// You should check out the gba-link-connection's original code instead of this.

// [!]
#pragma GCC push_options
#pragma GCC optimize("Os")

#ifdef LINK_WIRELESS_PUT_ISR_IN_IWRAM

LINK_CODE_IWRAM void LinkWireless::_onSerial() {
#ifdef LINK_WIRELESS_ENABLE_NESTED_IRQ
  interrupt = true;
  LINK_BARRIER;
  // (nested interrupts are enabled by LinkRawWireless::_onSerial(...))
#endif

  __onSerial();

#ifdef LINK_WIRELESS_ENABLE_NESTED_IRQ
  irqEnd();
#endif
}
void LinkWireless::_onTimer() {
#ifdef LINK_WIRELESS_ENABLE_NESTED_IRQ
  if (interrupt)
    return;

  interrupt = true;
  LINK_BARRIER;
  Link::_REG_IME = 1;
#endif

  __onTimer();

#ifdef LINK_WIRELESS_ENABLE_NESTED_IRQ
  irqEnd();
#endif
}

/**
 * NOTES:
 * When using `LINK_WIRELESS_ENABLE_NESTED_IRQ`:
 *   - Any user ISR can interrupt the library ISRs.
 *   - SERIAL ISR can interrupt TIMER ISR.
 *     -> This doesn't cause data races since TIMER ISR only works when
 *        there is no active async task.
 *     -> When TIMER ISR starts an async task (`transferAsync(...)`),
 *        nested interrupts are disabled (`REG_IME = 0`) and SERIAL cannot
 *        interrupt anymore.
 *   - TIMER interrupts are skipped if SERIAL ISR is running.
 *   - VBLANK interrupts are postponed if SERIAL or TIMER ISRs are running.
 *   - Nobody can interrupt VBLANK ISR.
 */

#endif

// [!]
#pragma GCC pop_options
