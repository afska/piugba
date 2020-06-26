#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include <libgba-sprite-engine/background/text_stream.h>

#define TEST_MODE false
#define STRESSTEST_MODE false
#define TIMINGTEST_MODE false
#define IFTEST if (TEST_MODE)
#define IFNOTTEST if (!TEST_MODE)
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
