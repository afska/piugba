#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

#ifndef ENV_DEBUG
// Defined in Makefile
#define ENV_DEBUG false
#endif

#ifndef ENV_DEVELOPMENT
// Defined in Makefile
#define ENV_DEVELOPMENT false
#endif

#ifndef ENV_ARCADE
// Defined in Makefile
#define ENV_ARCADE false
#endif

#define STRESSTEST_MODE false
#define TIMINGTEST_MODE false
#define IFSTRESSTEST if (STRESSTEST_MODE)
#define IFTIMINGTEST if (TIMINGTEST_MODE)
#define DSTR(EXP) std::to_string((EXP))
#define LOGN(NUM, LINE) (LOGSTR(DSTR(NUM).c_str(), LINE))
#define LOGSTR(STR, LINE) (TextStream::instance().setText(STR, 1 + LINE, 12))

#define DEBULOG(NUM) LOGN(NUM, -1)
static int DEBULIST_LINE = -1;
inline void DEBULIST(int num) {
  LOGN(num, DEBULIST_LINE);
  DEBULIST_LINE++;
}
#ifdef SENV_DEBUG
static int DEBUTRACE_LINE = -1;
#endif
inline void DEBUTRACE(std::string string) {
#ifdef SENV_DEBUG
  if (string.empty())
    return;

  if (DEBUTRACE_LINE == -1)
    TextStream::instance().clear();
  TextStream::instance().setText(string, 1 + DEBUTRACE_LINE, -3);
  DEBUTRACE_LINE++;
  if (DEBUTRACE_LINE == 17)
    DEBUTRACE_LINE = -1;
#endif
}

static bool BSOD_ON = false;

inline void BSOD(std::string message) {
  if (BSOD_ON)
    return;
  BSOD_ON = true;

  REG_RCNT |= 1 << 15;  // (disable link cable)
  player_stop();
  SCENE_init();
  BACKGROUND_enable(true, false, false, false);
  TextStream::instance().setText(message, 0, -3);
  while (true)
    ;
}

inline void BSOL(std::string message) {
  REG_RCNT |= 1 << 15;  // (disable link cable)
  player_stop();
  SCENE_init();
  BACKGROUND_enable(true, false, false, false);
  TextStream::instance().setText(message, 0, -3);
}

inline void profileStart() {
  REG_TM1CNT_L = 0;
  REG_TM2CNT_L = 0;

  REG_TM1CNT_H = 0;
  REG_TM2CNT_H = 0;

  REG_TM2CNT_H = TM_ENABLE | TM_CASCADE;
  REG_TM1CNT_H = TM_ENABLE | TM_FREQ_1;
}

inline u32 profileStop() {
  REG_TM1CNT_H = 0;
  REG_TM2CNT_H = 0;

  return (REG_TM1CNT_L | (REG_TM2CNT_L << 16));
}

inline u32 toMs(u32 cycles) {
  // CPU Frequency * time per frame = cycles per frame
  // 16780000 * (1/60) ~= 279666
  return (cycles * 1000) / (279666 * 60);
}

#endif  // DEBUG_TOOLS_H
