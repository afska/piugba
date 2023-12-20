#ifndef DARKENER_H
#define DARKENER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "gameplay/save/Settings.h"

class Darkener {
 public:
  Darkener(u8 id, u8 priority);

  void initialize(BackgroundType type);
  void initialize(BackgroundType type, u8 colorIndex);

 private:
  u8 id;
  u8 priority;
};

#endif  // DARKENER_H
