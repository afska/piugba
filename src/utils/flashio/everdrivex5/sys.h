/*
 * File:   sys.h
 * Author: krik
 *
 * Created on December 15, 2015, 1:12 PM
 */

#ifndef SYS_H
#define SYS_H

#pragma GCC system_header

#define u8 unsigned char
#define vu8 volatile unsigned char

#define u16 unsigned short
#define vu16 volatile unsigned short
#define u32 unsigned int
#define vu32 volatile unsigned int

void sysInit();
void sysFillRam(void* addr, u16 val, u32 len);
void gCleanScreen();
void gAppendString(u8* str);
void gConsPrint(u8* str);
void gSetXY(u8 x, u8 y);
void gAppendHex8(u8 num);
void gAppendHex16(u16 num);
void gSetPal(u16 pal);
void gVsync();
void sysInitColors();

void sysJoyWait();

#define RGB16(r, g, b) ((r) + (g << 5) + (b << 10))
#define GBA_DISPSTAT *((volatile u16*)0x4000004)
#define GBA_VCTR *((volatile u16*)0x4000006)
#define GBA_TIMER0_VAL *((vu16*)0x4000100)
#define GBA_TIMER0_CFG *((vu16*)0x4000102)
#define GBA_IO_RCNT *((vu16*)0x4000134)
#define GBA_IO_SIOCNT *((vu16*)0x4000128)
#define GBA_IO_SIODAT *((vu16*)0x400012A)

#define JOY_DELAY 32

#define JOY_B 0x0002
#define JOY_A 0x0001
#define JOY_SEL 0x0004
#define JOY_STA 0x0008
#define JOY_U 0x0040
#define JOY_D 0x0080
#define JOY_L 0x0020
#define JOY_R 0x0010

#define SYS_BR_ROWS 15

#define DMA_SRC *((vu32*)0x40000D4)
#define DMA_DST *((vu32*)0x40000D8)
#define DMA_LEN *((vu16*)0x40000DC)
#define DMA_CTR *((vu16*)0x40000DE)

#define PAL_BG_1 256

#define SCREEN_W 30
#define SCREEN_H 20
#define PLAN_W 32
#define PLAN_H 20
#define PLAN_SIZE (PLAN_W * PLAN_H * 2)

extern u16 joy;

#endif /* SYS_H */
