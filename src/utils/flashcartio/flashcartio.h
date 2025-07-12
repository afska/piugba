#ifndef FLASHCARTIO_H
#define FLASHCARTIO_H

#include <stdbool.h>
#include "fatfs/ff.h"

#define MAX_PATH_LENGTH 64  // [!]

typedef enum {
  NO_FLASHCART,
  EVERDRIVE_GBA_X5,
  EZ_FLASH_OMEGA,
  EMULATOR  // [!]
} ActiveFlashcart;

typedef enum {
  FLASHCART_ACTIVATED,
  NO_FLASHCART_FOUND,
  FLASHCART_ACTIVATION_FAILED
} ActivationResult;  // [!]

#define CAN_USE_BG_VIDEO_WITH_GSM (false)     // [!]
#define CAN_USE_HQ_AUDIO_DURING_MENUS (true)  // [!]

extern ActiveFlashcart active_flashcart;
extern volatile bool flashcartio_is_reading;
extern volatile bool flashcartio_needs_reset;     // [!]
extern void (*flashcartio_reset_callback)(void);  // [!]

ActivationResult flashcartio_activate(void);
bool flashcartio_read_sector(unsigned int sector,
                             unsigned char* destination,
                             unsigned short count);

// --- emulator FS --- // [!]
#define REG_FS_ENABLE (*(volatile unsigned short*)0x4FFF800)
#define REG_FS_FILENAME_LO (*(volatile unsigned short*)0x4FFF802)
#define REG_FS_FILENAME_HI (*(volatile unsigned short*)0x4FFF804)
#define REG_FS_OFFSET_LO (*(volatile unsigned short*)0x4FFF806)
#define REG_FS_OFFSET_HI (*(volatile unsigned short*)0x4FFF808)
#define REG_FS_SIZE (*(volatile unsigned short*)0x4FFF80A)
#define REG_FS_OUT_ADDRESS_LO (*(volatile unsigned short*)0x4FFF80C)
#define REG_FS_OUT_ADDRESS_HI (*(volatile unsigned short*)0x4FFF80E)
#define REG_FS_OUT_SUCCESS (*(volatile unsigned short*)0x4FFF810)

inline unsigned enableFs() {
  REG_FS_ENABLE = 0xF511;
  return REG_FS_ENABLE == 0x11F5;
}

inline bool isEmulatorFsEnabled() {
  return active_flashcart == EMULATOR;
}

inline int readFile(const char* fileName,
                    unsigned offset,
                    unsigned short size,
                    unsigned char* dest) {
  if (!enableFs())
    return -1;

  REG_FS_FILENAME_LO = (unsigned)fileName & 0xffff;
  REG_FS_FILENAME_HI = (unsigned)fileName >> 16;
  REG_FS_OFFSET_LO = offset & 0xffff;
  REG_FS_OFFSET_HI = offset >> 16;
  REG_FS_SIZE = size;
  REG_FS_OUT_ADDRESS_LO = (unsigned)dest & 0xffff;
  REG_FS_OUT_ADDRESS_HI = (unsigned)dest >> 16;
  REG_FS_OUT_SUCCESS = 0;  // (this write starts the read process)

  // (returns the number of read bytes, or -1)
  return REG_FS_OUT_SUCCESS == 1 ? REG_FS_SIZE : -1;
}

inline int getFileSize(const char* fileName) {
  if (!enableFs())
    return -1;

  REG_FS_FILENAME_LO = (unsigned)fileName & 0xffff;
  REG_FS_FILENAME_HI = (unsigned)fileName >> 16;
  REG_FS_OFFSET_LO = 0;
  REG_FS_OFFSET_HI = 0;
  REG_FS_SIZE = 0;
  REG_FS_OUT_ADDRESS_LO = 0;  // (`address == 0` means "retrieve file size")
  REG_FS_OUT_ADDRESS_HI = 0;
  REG_FS_OUT_SUCCESS = 0;

  return REG_FS_OUT_SUCCESS == 1
             ? (REG_FS_OUT_ADDRESS_HI << 16) | REG_FS_OUT_ADDRESS_LO
             : -1;
}
// --- emulator FS ---

#endif  // FLASHCARTIO_H
