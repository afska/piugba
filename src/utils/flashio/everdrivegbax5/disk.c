#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

#include "disk.h"
#include "bios.h"
#include "sys.h"

#define CMD0 0x40   // software reset
#define CMD1 0x41   // brings card out of idle state
#define CMD8 0x48   // Reserved
#define CMD12 0x4C  // stop transmission on multiple block read
#define CMD17 0x51  // read single block
#define CMD18 0x52  // read multiple block
#define CMD58 0x7A  // reads the OCR register
#define CMD55 0x77
#define CMD41 0x69
#define CMD24 0x58  // writes a single block
#define CMD25 0x59  // writes a multi block
#define ACMD41 0x69
#define ACMD6 0x46
#define SD_V2 2
#define SD_HC 1

#define CMD2 0x42  // read cid
#define CMD3 0x43  // read rca
#define CMD7 0x47
#define CMD9 0x49
#define CMD6 0x46

#define R1 1
#define R2 2
#define R3 3
#define R6 6
#define R7 7

u8 sd_resp_buff[18];
u8 card_type;
static u32 disk_addr;

u8 diskCloseRW();
void diskCrc16SD(void* src, u16* crc_out, u16 len);
u32 diskCrc7(u8* buff, u32 len);

const u16 crc_16_table[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
    0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
    0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
    0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
    0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
    0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
    0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
    0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
    0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
    0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
    0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
    0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
    0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
    0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
    0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
    0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
    0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
    0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
    0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
    0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
    0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
    0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

u32 diskCrc7(u8* buff, u32 len) {
  unsigned int a, crc = 0;

  while (len--) {
    crc ^= *buff++;
    a = 8;
    do {
      crc <<= 1;
      if (crc & (1 << 8))
        crc ^= 0x12;
    } while (--a);
  }
  return (crc & 0xfe);
}

u8 diskGetRespTypeSD(u8 cmd) {
  switch (cmd) {
    case CMD3:
      return R6;
    case CMD8:
      return R7;
    case CMD2:
    case CMD9:
      return R2;
    case CMD58:
    case CMD41:
      return R3;

    default:
      return R1;
  }
}

#define WAIT 2048

u8 diskCmdSD(u8 cmd, u32 arg) {
  u8 resp_type = diskGetRespTypeSD(cmd);
  u8 p = 0;
  u8 buff[6];
  volatile u32 i = 0;

  u8 resp_len = resp_type == R2 ? 17 : 6;
  u8 crc;
  buff[p++] = cmd;
  buff[p++] = (arg >> 24);
  buff[p++] = (arg >> 16);
  buff[p++] = (arg >> 8);
  buff[p++] = (arg >> 0);
  crc = diskCrc7(buff, 5) | 1;

  // biSDcmdWriteMode(0);
  // bi_sd_mode(SPI_MODE_CMD_W4);
  bi_sd_mode(SD_MODE8);

  bi_sd_cmd_wr(0xff);
  bi_sd_cmd_wr(cmd);
  bi_sd_cmd_wr(arg >> 24);
  bi_sd_cmd_wr(arg >> 16);
  bi_sd_cmd_wr(arg >> 8);
  bi_sd_cmd_wr(arg);
  bi_sd_cmd_wr(crc);

  if (cmd == CMD18)
    return 0;

  bi_sd_cmd_rd();
  bi_sd_mode(SD_MODE1);
  i = 0;
  for (;;) {
    if ((bi_sd_cmd_val() & 192) == 0)
      break;
    // if (bi_sd_cmd_val() != 0xff)break;
    // if ((bi_sd_cmd_val() & 192) != 0)break;
    if (i++ == WAIT)
      return DISK_ERR_CMD_TIMEOUT;
    bi_sd_cmd_rd();
  }

  bi_sd_mode(SD_MODE8);
  // bi_sd_mode(SPI_MODE_CMD_R4);

  sd_resp_buff[0] = bi_sd_cmd_rd();

  for (i = 1; i < resp_len - 1; i++) {
    sd_resp_buff[i] = bi_sd_cmd_rd();
  }
  sd_resp_buff[i] = bi_sd_cmd_val();

  if (resp_type != R3) {
    if (resp_type == R2) {
      crc = diskCrc7(sd_resp_buff + 1, resp_len - 2) | 1;
    } else {
      crc = diskCrc7(sd_resp_buff, resp_len - 1) | 1;
    }
    if (crc != sd_resp_buff[resp_len - 1])
      return DISK_ERR_CRC_ERROR;
  }

  return 0;
}

u8 diskInit() {
  u16 i;
  volatile u8 resp = 0;
  u32 rca;
  u32 wait_len = WAIT;

  card_type = 0;

  bi_sd_speed(SD_SPD_LO);

  bi_sd_mode(SD_MODE8);

  for (i = 0; i < 40; i++)
    bi_sd_dat_wr(0xff);
  resp = diskCmdSD(CMD0, 0x1aa);

  for (i = 0; i < 40; i++)
    bi_sd_dat_wr(0xff);

  resp = diskCmdSD(CMD8, 0x1aa);

  if (resp != 0 && resp != DISK_ERR_CMD_TIMEOUT) {
    return DISK_ERR_INIT + 0;
  }
  if (resp == 0)
    card_type |= SD_V2;

  if (card_type == SD_V2) {
    for (i = 0; i < wait_len; i++) {
      resp = diskCmdSD(CMD55, 0);
      if (resp)
        return DISK_ERR_INIT + 1;
      if ((sd_resp_buff[3] & 1) != 1)
        continue;
      resp = diskCmdSD(CMD41, 0x40300000);
      if ((sd_resp_buff[1] & 128) == 0)
        continue;

      break;
    }
  } else {
    i = 0;
    do {
      resp = diskCmdSD(CMD55, 0);
      if (resp)
        return DISK_ERR_INIT + 2;
      resp = diskCmdSD(CMD41, 0x40300000);
      if (resp)
        return DISK_ERR_INIT + 3;

    } while (sd_resp_buff[1] < 1 && i++ < wait_len);
  }

  if (i == wait_len)
    return DISK_ERR_INIT + 4;

  if ((sd_resp_buff[1] & 64) && card_type != 0)
    card_type |= SD_HC;

  resp = diskCmdSD(CMD2, 0);
  if (resp)
    return DISK_ERR_INIT + 5;

  resp = diskCmdSD(CMD3, 0);
  if (resp)
    return DISK_ERR_INIT + 6;

  resp = diskCmdSD(CMD7, 0);

  rca = (sd_resp_buff[1] << 24) | (sd_resp_buff[2] << 16) |
        (sd_resp_buff[3] << 8) | (sd_resp_buff[4] << 0);

  resp = diskCmdSD(CMD9, rca);  // get csd
  if (resp)
    return DISK_ERR_INIT + 7;

  resp = diskCmdSD(CMD7, rca);
  if (resp)
    return DISK_ERR_INIT + 8;

  resp = diskCmdSD(CMD55, rca);
  if (resp)
    return DISK_ERR_INIT + 9;

  resp = diskCmdSD(CMD6, 2);
  if (resp)
    return DISK_ERR_INIT + 10;

  bi_sd_speed(SD_SPD_HI);

  disk_addr = ~0;

  return 0;
}

u8 diskOpenRead(u32 saddr) {
  u8 resp;
  if ((card_type & SD_HC) == 0)
    saddr *= 512;

  resp = diskCmdSD(CMD18, saddr);
  if (resp)
    return DISK_ERR_RD1;

  return 0;
}

u8 diskRead(u32 sd_addr, u8* dst, u16 slen) {
  u8 resp;
  if (sd_addr != disk_addr) {
    resp = diskCloseRW();
    if (resp)
      return resp;
    resp = diskOpenRead(sd_addr);
    if (resp)
      return resp;
    disk_addr = sd_addr;
  }

  resp = bi_sd_dma_rd(dst, slen);
  if (resp)
    return DISK_ERR_RD2;

  disk_addr += slen;

  return 0;
}

u8 diskCloseRW() {
  u8 resp;
  u16 i;

  if (disk_addr == ~0)
    return 0;
  disk_addr = ~0;
  resp = diskCmdSD(CMD12, 0);
  if (resp)
    return DISK_ERR_CLOSE_RW1;

  // gVsync();
  // gVsync();

  bi_sd_mode(SD_MODE1);
  bi_sd_dat_rd();
  bi_sd_dat_rd();
  bi_sd_dat_rd();
  bi_sd_mode(SD_MODE2);

  i = 65535;
  while (--i) {
    if (bi_sd_dat_rd() == 0xff)
      break;
  }
  if (i == 0)
    return DISK_ERR_CLOSE_RW2;

  return 0;
}

void diskPowerDown() {
  diskCmdSD(CMD0, 0x1aa);
}

void crc16SD_HW(u16* crc_out) {
  u16 u;
  u16 i;
  u16 crc_table[4];
  u16 tmp;

  u8 buff[512];
  bi_sd_read_crc_ram(buff);

  for (i = 0; i < 4; i++)
    crc_table[i] = 0;

  u8* data_ptr0 = &buff[0];
  u8* data_ptr1 = &buff[128];
  u8* data_ptr2 = &buff[256];
  u8* data_ptr3 = &buff[384];

  for (i = 0; i < 128; i++) {
    tmp = crc_table[0];
    crc_table[0] = crc_16_table[(tmp >> 8) ^ *data_ptr0++];
    crc_table[0] = crc_table[0] ^ (tmp << 8);

    tmp = crc_table[1];
    crc_table[1] = crc_16_table[(tmp >> 8) ^ *data_ptr1++];
    crc_table[1] = crc_table[1] ^ (tmp << 8);

    tmp = crc_table[2];
    crc_table[2] = crc_16_table[(tmp >> 8) ^ *data_ptr2++];
    crc_table[2] = crc_table[2] ^ (tmp << 8);

    tmp = crc_table[3];
    crc_table[3] = crc_16_table[(tmp >> 8) ^ *data_ptr3++];
    crc_table[3] = crc_table[3] ^ (tmp << 8);
  }

  for (i = 0; i < 4; i++) {
    for (u = 0; u < 16; u++) {
      crc_out[3 - i] >>= 1;
      crc_out[3 - i] |= (crc_table[u % 4] & 1) << 15;
      crc_table[u % 4] >>= 1;
    }
  }
}

u8 diskWrite(u32 sd_addr, void* src, u16 slen) {
  u8 resp;
  u16 crc16[5];
  u16 i;
  u16 u;
  u32 saddr = sd_addr;

  resp = diskCloseRW();
  if (resp)
    return resp;

  disk_addr = sd_addr;
  if ((card_type & 1) == 0)
    saddr *= 512;

  resp = diskCmdSD(CMD25, saddr);
  if (resp)
    return DISK_ERR_WR1;

  while (slen--) {
    bi_sd_mode(SD_MODE2);
    bi_sd_dat_wr(0xff);
    bi_sd_dat_wr(0xf0);

    bi_sd_dma_wr(src);
    crc16SD_HW(crc16);
    src += 512;
    // bi_sd_mode(SD_MODE4);
    // for (i = 0; i < 256; i++)bi_sd_dat_wr16(*buff++);

    bi_sd_mode(SD_MODE2);
    for (i = 0; i < 4; i++) {
      // bi_sd_dat_wr16(crc16[i]);
      bi_sd_dat_wr(crc16[i] >> 8);
      bi_sd_dat_wr(crc16[i] & 0xff);
    }

    bi_sd_mode(SD_MODE1);
    bi_sd_dat_wr(0xff);

    bi_sd_dat_rd();  // new

    i = 1024;
    while ((bi_sd_dat_rd() & 1) != 0 && --i != 0)
      ;
    if (i == 0)
      return DISK_ERR_WR3;  // timeout
    resp = 0;

    for (i = 0; i < 3; i++) {
      resp <<= 1;
      u = bi_sd_dat_rd();
      resp |= u & 1;
    }
    resp &= 7;

    if (resp != 0x02) {
      if (resp == 5)
        return DISK_ERR_WR4;  // crc error
      return DISK_ERR_WR5;
    }

    bi_sd_mode(SD_MODE1);
    bi_sd_dat_rd();

    i = 65535;
    while (--i) {
      if (bi_sd_dat_rd() == 0xff)
        break;
    }

    if (i == 0)
      return DISK_ERR_WR2;
  }

  resp = diskCloseRW();
  if (resp)
    return resp;

  return 0;
}
