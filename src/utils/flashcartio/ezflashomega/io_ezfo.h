#ifndef IO_EZFO_H
#define IO_EZFO_H

#pragma GCC system_header

#include "../sys.h"

bool _EZFO_startUp(void);
bool _EZFO_readSectors(u32 address, u32 count, void* buffer);

#endif /* IO_EZFO_H */
