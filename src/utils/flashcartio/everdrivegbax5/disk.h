/*
 * File:   disk.h
 * Author: krik
 *
 * Created on December 19, 2015, 1:24 PM
 */

// EverDrive-GBA IO 05.09.16

#ifndef DISK_H
#define DISK_H

#pragma GCC system_header

#include <tonc_core.h>

#define DISK_ERR_INIT 0xC0
#define DISK_ERR_RD1 0xD2
#define DISK_ERR_RD2 0xD3

#define DISK_ERR_WR1 0xD4
#define DISK_ERR_WR2 0xD5
#define DISK_ERR_WR3 0xD6  // timeout
#define DISK_ERR_WR4 0xD7  // crc error
#define DISK_ERR_WR5 0xD8

#define DISK_ERR_CMD_TIMEOUT 0xD9
#define DISK_ERR_CRC_ERROR 0xDA
#define DISK_ERR_CLOSE_RW1 0xDB
#define DISK_ERR_CLOSE_RW2 0xDC

u8 diskInit();
u8 diskRead(u32 sd_addr, u8* dst, u16 slen);
u8 diskWrite(u32 sd_addr, void* src, u16 slen);
void diskPowerDown();

#endif /* DISK_H */
