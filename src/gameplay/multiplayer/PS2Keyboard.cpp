#include "PS2Keyboard.h"

#define CODE_EWRAM __attribute__((section(".ewram")))

CODE_EWRAM void PS2Keyboard::_onSerial() {
  if (!isEnabled)
    return;

  u8 val = (REG_RCNT & PS2_SO_DATA) != 0;

  u32 nowFrame = frameCounter;
  if (nowFrame - prevFrame > PS2_TIMEOUT_FRAMES) {
    bitcount = 0;
    incoming = 0;
  }
  prevFrame = nowFrame;

  u8 n = bitcount - 1;
  if (n <= 7)
    incoming |= (val << n);
  bitcount++;

  if (bitcount == 11) {
    process(incoming);

    bitcount = 0;
    incoming = 0;
  }
}
