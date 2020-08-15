#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include <libgba-sprite-engine/background/text_stream.h>

#ifndef ENABLE_STAGE_BREAK
// Defined in Makefile: Show fail screen when lifebar is empty
#define ENABLE_STAGE_BREAK true
#endif

#ifndef ENABLE_BACKGROUND
// Defined in Makefile: Show background in game
#define ENABLE_BACKGROUND true
#endif

#ifndef IGNORE_LOCKS
// Defined in Makefile: Allow playing locked songs
#define IGNORE_LOCKS false
#endif

#define STRESSTEST_MODE false
#define TIMINGTEST_MODE false
#define IFSTRESSTEST if (STRESSTEST_MODE)
#define IFTIMINGTEST if (TIMINGTEST_MODE)
#define LOGN(NUM, LINE) (LOGSTR(std::to_string(NUM).c_str(), LINE))
#define LOGSTR(STR, LINE) (TextStream::instance().setText(STR, 1 + LINE, 15))
#define DEBULOG(NUM) LOGN(NUM, -1)
static int DEBULIST_LINE = -1;
inline void DEBULIST(int num) {
  LOGN(num, DEBULIST_LINE);
  DEBULIST_LINE++;
}

#endif  // DEBUG_TOOLS_H
