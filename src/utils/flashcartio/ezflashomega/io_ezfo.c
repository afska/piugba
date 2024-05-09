/*
  io_ezfo.c
  Hardware Routines for reading the EZ Flash Omega filesystem
*/

#include "io_ezfo.h"

// SOURCE:
// https://github.com/ez-flash/omega-de-kernel/blob/main/source/Ezcard_OP.c

// [!] Some small optimizations were made for piuGBA

// --------------------------------------------------------------------
inline __attribute__((always_inline)) void SetSDControl(u16 control) {
  *(vu16*)0x9fe0000 = 0xd200;
  *(vu16*)0x8000000 = 0x1500;
  *(vu16*)0x8020000 = 0xd200;
  *(vu16*)0x8040000 = 0x1500;
  *(vu16*)0x9400000 = control;
  *(vu16*)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) void SD_Enable(void) {
  SetSDControl(1);
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) void SD_Read_state(void) {
  SetSDControl(3);
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) void SD_Disable(void) {
  SetSDControl(0);
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) u16 SD_Response(void) {
  return *(vu16*)0x9E00000;
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) u32 Wait_SD_Response() {
  vu16 res;
  u32 count = 0;
  while (1) {
    res = SD_Response();
    if (res != 0xEEE1) {
      return 0;
    }

    count++;
    if (count > 0x100000)
      return 1;
  }
}
// --------------------------------------------------------------------
inline __attribute__((always_inline)) u32 Read_SD_sectors(u32 address,
                                                          u16 count,
                                                          u8* SDbuffer) {
  SD_Enable();

  u16 i;
  u16 blocks;
  u32 res;
  for (i = 0; i < count; i += 4) {
    blocks = (count - i > 4) ? 4 : (count - i);

    *(vu16*)0x9fe0000 = 0xd200;
    *(vu16*)0x8000000 = 0x1500;
    *(vu16*)0x8020000 = 0xd200;
    *(vu16*)0x8040000 = 0x1500;
    *(vu16*)0x9600000 = ((address + i) & 0x0000FFFF);
    *(vu16*)0x9620000 = ((address + i) & 0xFFFF0000) >> 16;
    *(vu16*)0x9640000 = blocks;
    *(vu16*)0x9fc0000 = 0x1500;
    SD_Read_state();
    res = Wait_SD_Response();
    SD_Enable();
    if (res == 1) {
      SD_Disable();
      return 1;
    }

    dmaCopy((void*)0x9E00000, SDbuffer + i * 512, blocks * 512);
  }
  SD_Disable();
  return 0;
}
// --------------------------------------------------------------------
__attribute__((section(".iwram"), target("arm"), noinline)) void SetRompage(
    u16 page) {
  *(vu16*)0x9fe0000 = 0xd200;
  *(vu16*)0x8000000 = 0x1500;
  *(vu16*)0x8020000 = 0xd200;
  *(vu16*)0x8040000 = 0x1500;
  *(vu16*)0x9880000 = page;  // C4
  *(vu16*)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------

#define ROMPAGE_BOOTLOADER 0x8000
#define ROMPAGE_PSRAM 0x200
#define S98WS512PE0_FLASH_PAGE_MAX 0x200
#define ROM_HEADER_CHECKSUM *(vu16*)(0x8000000 + 188)

static u16 EWRAM_BSS ROMPAGE_ROM;

// returns true if the data is a match
static bool EWRAM_CODE _EZFO_TestRompage(u16 wanted, u16 page) {
  SetRompage(page);
  if (wanted == ROM_HEADER_CHECKSUM) {
    ROMPAGE_ROM = page;
    return true;
  }
  return false;
}

bool EWRAM_CODE _EZFO_startUp(void) {
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
  u16 ime = REG_IME;
  REG_IME = 0;
#endif
  const u16 complement = ROM_HEADER_CHECKSUM;

  // unmap rom, if the data matches, then this is not an ezflash
  if (_EZFO_TestRompage(complement, ROMPAGE_BOOTLOADER)) {
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
    REG_IME = ime;
#endif
    return false;
  }

  // find where the rom is mapped, try psram first
  if (_EZFO_TestRompage(complement, ROMPAGE_PSRAM)) {
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
    REG_IME = ime;
#endif
    return true;
  }

  // try and find it within norflash, test each 1MiB page (512 pages)
  for (int i = 0; i < S98WS512PE0_FLASH_PAGE_MAX; i++) {
    if (_EZFO_TestRompage(complement, i)) {
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
      REG_IME = ime;
#endif
      return true;
    }
  }

#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
  REG_IME = ime;
#endif

  // this literally shouldn't happen, contact me if you hit this!
  return false;
}

bool EWRAM_CODE _EZFO_readSectors(u32 address, u32 count, void* buffer) {
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
  u16 ime = REG_IME;
  REG_IME = 0;
#endif
  SetRompage(ROMPAGE_BOOTLOADER);
  const u32 result = Read_SD_sectors(address, count, buffer);
  SetRompage(ROMPAGE_ROM);
#if FLASHCARTIO_EZFO_DISABLE_IRQ != 0
  REG_IME = ime;
#endif
  return result == 0;
}
