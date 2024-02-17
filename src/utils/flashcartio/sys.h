#ifndef SYS_H
#define SYS_H

#include <stdbool.h>

// Use DMA1 instead of DMA3 (uncomment to enable)
#define FLASHCARTIO_USE_DMA1

#ifdef FLASHCARTIO_USE_DMA1
#define DMA_SRC *((volatile u32*)0x40000BC)
#define DMA_DST *((volatile u32*)0x40000C0)
#define DMA_LEN *((volatile u16*)0x40000C4)
#define DMA_CTR *((volatile u16*)0x40000C6)
#endif
#ifndef FLASHCARTIO_USE_DMA1
#define DMA_SRC *((vu32*)0x40000D4)
#define DMA_DST *((vu32*)0x40000D8)
#define DMA_LEN *((vu16*)0x40000DC)
#define DMA_CTR *((vu16*)0x40000DE)
#endif

#define u8 unsigned char
#define vu8 volatile unsigned char
#define u16 unsigned short
#define vu16 volatile unsigned short
#define u32 unsigned int
#define vu32 volatile unsigned int

#define EWRAM_CODE __attribute__((section(".ewram"), long_call))
#define EWRAM_BSS __attribute__((section(".sbss")))

#define REG_BASE_ 0x04000000
#define REG_DMA0SAD *(vu32*)(REG_BASE_ + 0x0b0)
#define REG_DMA0DAD *(vu32*)(REG_BASE_ + 0x0b4)
#define REG_DMA0CNT *(vu32*)(REG_BASE_ + 0x0b8)
#define REG_DMA1SAD *(vu32*)(REG_BASE_ + 0x0bc)
#define REG_DMA1DAD *(vu32*)(REG_BASE_ + 0x0c0)
#define REG_DMA1CNT *(vu32*)(REG_BASE_ + 0x0c4)
#define REG_DMA2SAD *(vu32*)(REG_BASE_ + 0x0c8)
#define REG_DMA2DAD *(vu32*)(REG_BASE_ + 0x0cc)
#define REG_DMA2CNT *(vu32*)(REG_BASE_ + 0x0d0)
#define REG_DMA3SAD *(vu32*)(REG_BASE_ + 0x0d4)
#define REG_DMA3DAD *(vu32*)(REG_BASE_ + 0x0d8)
#define REG_DMA3CNT *(vu32*)(REG_BASE_ + 0x0dc)
#define DMA_DST_INC (0 << 21)
#define DMA_DST_DEC (1 << 21)
#define DMA_DST_FIXED (2 << 21)
#define DMA_DST_RELOAD (3 << 21)
#define DMA_SRC_INC (0 << 23)
#define DMA_SRC_DEC (1 << 23)
#define DMA_SRC_FIXED (2 << 23)
#define DMA_REPEAT (1 << 25)
#define DMA16 (0 << 26)
#define DMA32 (1 << 26)
#define GAMEPAK_DRQ (1 << 27)
#define DMA_IMMEDIATE (0 << 28)
#define DMA_VBLANK (1 << 28)
#define DMA_HBLANK (2 << 28)
#define DMA_SPECIAL (3 << 28)
#define DMA_IRQ (1 << 30)
#define DMA_ENABLE (1 << 31)

#define DMA_Copy(channel, source, dest, mode)    \
  {                                              \
    REG_DMA##channel##SAD = (u32)(source);       \
    REG_DMA##channel##DAD = (u32)(dest);         \
    REG_DMA##channel##CNT = DMA_ENABLE | (mode); \
  }

inline __attribute__((always_inline)) void dmaCopy(const void* source,
                                                   void* dest,
                                                   u32 size) {
#ifdef FLASHCARTIO_USE_DMA1
  DMA_Copy(1, source, dest, DMA16 | size >> 1);
#endif
#ifndef FLASHCARTIO_USE_DMA1
  DMA_Copy(3, source, dest, DMA16 | size >> 1);
#endif
}

#endif /* SYS_H */
