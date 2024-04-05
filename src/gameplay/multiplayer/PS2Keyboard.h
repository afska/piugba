#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

// A PS/2 Keyboard integration for piuGBA
//  ____________
// |   Pinout   |
// |PS/2 --- GBA|
// |------------|
// |CLOCK -> SI |
// |DATA --> SO |
// |VCC ---> VCC|
// |GND ---> GND|

#include <tonc_core.h>

#define PS2_KEY_P1_DOWNLEFT 26    // Z
#define PS2_KEY_P1_UPLEFT 21      // Q
#define PS2_KEY_P1_CENTER 27      // S
#define PS2_KEY_P1_UPRIGHT 36     // E
#define PS2_KEY_P1_DOWNRIGHT 33   // C
#define PS2_KEY_P2_DOWNLEFT 105   // Numpad 1
#define PS2_KEY_P2_UPLEFT 108     // Numpad 7
#define PS2_KEY_P2_CENTER 115     // Numpad 5
#define PS2_KEY_P2_UPRIGHT 125    // Numpad 9
#define PS2_KEY_P2_DOWNRIGHT 122  // Numpad 3
#define PS2_KEY_START_1 90        // Enter
#define PS2_KEY_START_2 121       // Numpad +
#define PS2_KEY_SELECT_1 102      // Backspace
#define PS2_KEY_SELECT_2 123      // Numpad -
#define PS2_KEY_LEFT 107          // Left
#define PS2_KEY_RIGHT 116         // Right
#define PS2_KEY_UP 117            // Up
#define PS2_KEY_DOWN 114          // Down
#define PS2_KEY_ESC 118           // ESC
#define PS2_KEY_SUPR 113          // Supr

#define PS2_KEY_RELEASE 240
#define PS2_KEY_SPECIAL 224
#define PS2_SO_DATA 0b1000
#define PS2_TIMEOUT_FRAMES 15  // (~250ms)

class PS2Keyboard {
 public:
  bool arrows[10];
  bool start1, start2, select1, select2, left, right, up, down;
  bool softReset = false;

  bool any() {
    for (u32 i = 0; i < 10; i++)
      if (arrows[i])
        return true;
    return start1 || start2 || select1 || select2 || left || right || up ||
           down;
  }

  bool isActive() { return isEnabled; }

  void activate() {
    reset();
    isEnabled = true;
  }

  void deactivate() {
    isEnabled = false;
    reset();
  }

  void _onVBlank() {
    if (!isEnabled)
      return;

    frameCounter++;
  }

  void _onSerial();

 private:
  volatile bool isEnabled = false;
  u8 bitcount = 0;
  u8 incoming = 0;
  u32 prevFrame = 0;
  u32 frameCounter = 0;
  bool isRelease = false;
  bool isSpecial = false;
  u8 parityBit = 0;

  bool process(u8 scanCode) {
    switch (scanCode) {
      case PS2_KEY_P1_DOWNLEFT:
        return assign(arrows[0]);
      case PS2_KEY_P1_UPLEFT:
        return assign(arrows[1]);
      case PS2_KEY_P1_CENTER:
        return assign(arrows[2]);
      case PS2_KEY_P1_UPRIGHT:
        return assign(arrows[3]);
      case PS2_KEY_P1_DOWNRIGHT:
        return assign(arrows[4]);
      case PS2_KEY_P2_DOWNLEFT:
        return assign(arrows[5]);
      case PS2_KEY_P2_UPLEFT:
        return assign(arrows[6]);
      case PS2_KEY_P2_CENTER:
        return assign(arrows[7]);
      case PS2_KEY_P2_UPRIGHT:
        return assign(arrows[8]);
      case PS2_KEY_P2_DOWNRIGHT:
        return assign(arrows[9]);
      case PS2_KEY_START_1:
        return assign(start1);
      case PS2_KEY_START_2:
        return assign(start2);
      case PS2_KEY_SELECT_1:
        return assign(select1);
      case PS2_KEY_SELECT_2:
        return assign(select2);
      case PS2_KEY_LEFT:
        if (isSpecial)
          return assign(left);
        else
          break;
      case PS2_KEY_RIGHT:
        if (isSpecial)
          return assign(right);
        else
          break;
      case PS2_KEY_UP:
        if (isSpecial)
          return assign(up);
        else
          break;
      case PS2_KEY_DOWN:
        if (isSpecial)
          return assign(down);
        else
          break;
      case PS2_KEY_ESC: {
        return (softReset = true);
      }
      case PS2_KEY_SUPR: {
        if (isSpecial) {
          softReset = true;
          return true;
        } else
          break;
      }
      case PS2_KEY_RELEASE: {
        isRelease = true;
        return true;
      }
      case PS2_KEY_SPECIAL: {
        isSpecial = true;
        return true;
      }
      default: {
      }
    }
    isRelease = false;
    isSpecial = false;
    return false;
  }

  bool assign(bool& output) {
    output = !isRelease;
    isRelease = false;
    isSpecial = false;
    return true;
  }

  void reset() {
    for (u32 i = 0; i < 10; i++)
      arrows[i] = false;
    start1 = start2 = select1 = select2 = left = right = up = false;

    bitcount = 0;
    incoming = 0;
    prevFrame = 0;
    frameCounter = 0;
    isRelease = false;
    isSpecial = false;
    parityBit = 0;
  }
};

extern PS2Keyboard* ps2Keyboard;

inline void PS2_ISR_VBLANK() {
  ps2Keyboard->_onVBlank();
}

inline void PS2_ISR_SERIAL() {
  ps2Keyboard->_onSerial();
}

#endif  // PS2_KEYBOARD_H
