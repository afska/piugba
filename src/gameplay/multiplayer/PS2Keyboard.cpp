#include "PS2Keyboard.h"

#define CODE_EWRAM __attribute__((section(".ewram")))

CODE_EWRAM void PS2Keyboard::_onSerial() {
  if (!isEnabled)
    return;

  u8 val = (REG_RCNT & PS2_SO_DATA) != 0;

  u32 nowFrame = frameCounter;
  if (nowFrame - prevFrame > PS2_TIMEOUT_FRAMES) {  // 250ms timeout
    bitcount = 0;
    incoming = 0;
    parityBit = 0;
  }
  prevFrame = nowFrame;

  if (bitcount == 0 && val == 0) {  // start bit detected
    // start bit is always 0, so only proceed if val is 0
    bitcount++;
  } else if (bitcount >= 1 && bitcount <= 8) {  // data bits
    incoming |= (val << (bitcount - 1));
    bitcount++;
  } else if (bitcount == 9) {  // parity bit
    // store parity bit for later check
    parityBit = val;
    bitcount++;
  } else if (bitcount == 10) {  // stop bit
    if (val == 1) {             // stop bit should be 1
      // calculate parity (including the stored parity bit from previous IRQ)
      u8 parity = 0;
      for (u8 i = 0; i < 8; i++)
        parity += (incoming >> i) & 1;
      parity += parityBit;

      if (parity % 2 != 0)  // odd parity as expected
        process(incoming);
    }
    bitcount = 0;
    incoming = 0;
    parityBit = 0;
  }
}
