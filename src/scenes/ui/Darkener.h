#ifndef DARKENER_H
#define DARKENER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

class Darkener {
 public:
  Darkener(u8 id);

  void initialize();

 private:
  u8 id;
};

#endif  // DARKENER_H
