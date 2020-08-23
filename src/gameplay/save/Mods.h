#ifndef MODS_H
#define MODS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum PixelateOpts : u8 { pOFF, pLIFE, pFIXED, pBLINK, pRANDOM };
enum ReduceOpts : u8 { rOFF, rFIXED, rRANDOM };
enum NegativeOpts : u8 { nOFF, nFIXED, tTOGGLE };

typedef struct __attribute__((__packed__)) {
  u8 multiplier;
  u8 stageBreak;
  PixelateOpts pixelate;
  u8 jump;
  ReduceOpts reduce;
  NegativeOpts negative;
  u8 randomSpeed;
  u8 mirrorSteps;
  u8 randomSteps;
  u8 extraJudgement;
} Mods;

#endif  // MODS_H
