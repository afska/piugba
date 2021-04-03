#ifndef ADMIN_SETTINGS_H
#define ADMIN_SETTINGS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum ArcadeChartsOpts : u8 { SINGLE, DOUBLE };
enum RumbleOpts : u8 { RUMBLE_OFF, RUMBLE_LOW, RUMBLE_HIGH };
enum IOBlinkOpts : u8 {
  IO_BLINK_OFF,
  IO_BLINK_ON_BEAT,
  IO_BLINK_ON_HIT,
  IO_BLINK_ON_KEY
};
enum SRAMBlinkOpts : u8 {
  SRAM_BLINK_OFF,
  SRAM_BLINK_ON_BEAT,
  SRAM_BLINK_ON_HIT,
  SRAM_BLINK_ON_KEY
};

typedef struct __attribute__((__packed__)) {
  u8 arcadeCharts;
  u8 rumble;
  u8 ioBlink;
  u8 sramBlink;
  u8 beat;
} AdminSettings;

#endif  // ADMIN_SETTINGS_H
