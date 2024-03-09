#ifndef DARKENER_H
#define DARKENER_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_memmap.h>

#include "gameplay/save/Settings.h"

class Darkener {
 public:
  Darkener(u8 id, u8 priority);

  void initialize(BackgroundType type);
  void initialize(BackgroundType type, u8 colorIndex);
  void setX(int x) { this->x = x; }
  void render() { REG_BG_OFS[id].x = x; }

 private:
  u8 id;
  u8 priority;
  int x = 0;
};

#endif  // DARKENER_H
