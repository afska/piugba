#ifndef MODS_H
#define MODS_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum StageBreakOpts : u8 { sON, sOFF, sSUDDEN_DEATH };
enum PixelateOpts : u8 { pOFF, pLIFE, pFIXED, pBLINK_IN, pBLINK_OUT, pRANDOM };
enum JumpOpts : u8 { jOFF, jLINEAR, jRANDOM };
enum ReduceOpts : u8 { rOFF, rLINEAR, rFIXED, rRANDOM, rMICRO };
enum DecolorizeOpts : u8 { dOFF, dINVERT, dGRAY, dRED, dGREEN, dBLUE };
enum TrainingModeOpts : u8 { tOFF, tON, tSILENT };

typedef struct __attribute__((__packed__)) {
  u8 multiplier;
  StageBreakOpts stageBreak;
  PixelateOpts pixelate;
  JumpOpts jump;
  ReduceOpts reduce;
  DecolorizeOpts decolorize;
  u8 randomSpeed;
  u8 mirrorSteps;
  u8 randomSteps;
  TrainingModeOpts trainingMode;
} Mods;

#endif  // MODS_H
