/*
 * File:   bios.h
 * Author: krik
 *
 * Created on December 15, 2015, 1:50 PM
 */

#ifndef BIOS_H
#define BIOS_H

#pragma GCC system_header

#include "../sys.h"

#define BI_USB_BUFF 32767

#define SD_SPD_LO 0
#define SD_SPD_HI 1

#define SD_MODE1 0
#define SD_MODE2 2
#define SD_MODE4 4
#define SD_MODE8 6

#define BI_SAV_EEP 16
#define BI_SAV_SRM 32
#define BI_SAV_FLA64 64
#define BI_SAV_FLA128 80
#define BI_SAV_BITS (BI_SAV_EEP | BI_SAV_FLA128 | BI_SAV_FLA64 | BI_SAV_SRM)
#define BI_RAM_BNK_0 0
#define BI_RAM_BNK_1 128
#define BI_RAM_BNK_2 256
#define BI_RAM_BNK_3 384

#define BI_CART_FEA_RTC 0x0001
#define BI_CART_FEA_SPD 0x0002
#define BI_CART_FEA_BAT 0x0004
#define BI_CART_FEA_CRC_RAM 0x0008
#define BI_CART_TYPE 0xff00

#define EEP_SIZE_512 6
#define EEP_SIZE_8K 14

#define GBA_WAITCNT *((vu32*)0x4000204)
#define GBA_TIMER1_VAL *((vu16*)0x4000104)
#define GBA_TIMER1_CFG *((vu16*)0x4000106)
#define IRQ_ACK_REG *(vu16*)0x4000202
#define IRQ_GAME_PAK 0x2000

bool bi_init_sd_only();
void bi_init();
void bi_lock_regs();
void bi_unlock_regs();
void bi_dma_mem(void* src, void* dst, int len);

void bi_sd_cmd_wr(u8 data);
u8 bi_sd_cmd_rd();
void bi_sd_dat_wr(u8 data);
u8 bi_sd_dat_rd();
u8 bi_sd_dma_rd(void* dst, int slen);

void bi_sd_mode(u8 mode);
void bi_sd_speed(u8 speed);

u8 bi_eep_write(void* src, u16 addr, u16 len);
u8 bi_eep_read(void* dst, u16 addr, u16 len);
void bi_set_eep_size(u8 size);
void bi_set_save_type(u8 save_type);

u16 bi_flash_id();
void bi_flash_erase_chip();
void bi_flash_erase_sector(u8 sector);
void bi_flash_write(void* src, u32 addr, u32 len);
void bi_flash_set_bank(u8 bank);

void bi_sram_read(void* dst, u32 offset, u32 len);
void bi_sram_write(void* src, u32 offset, u32 len);
u16 bi_get_fpga_ver();

void bi_rtc_on();
void bi_rtc_off();

void bi_set_rom_bank(u8 bank);
void bi_set_rom_mask(u32 rom_size);

#endif /* BIOS_H */
