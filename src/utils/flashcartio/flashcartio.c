#include "flashcartio.h"

#include "everdrivegbax5/bios.h"
#include "everdrivegbax5/disk.h"
#include "ezflashomega/io_ezfo.h"

ActiveFlashcart active_flashcart = NO_FLASHCART;

bool flashcartio_activate(void) {
  // Everdrive GBA X5
  if (bi_init_sd_only() && diskInit() == 0) {
    active_flashcart = EVERDRIVE_GBA_X5;
    return true;
  }

  // EZ Flash Omega
  if (_EZFO_startUp()) {
    active_flashcart = EZ_FLASH_OMEGA;
    return true;
  }

  return false;
}

bool flashcartio_read_sector(u32 sector, u8* destination, u16 count) {
  switch (active_flashcart) {
    case EVERDRIVE_GBA_X5: {
      return diskRead(sector, destination, count) == 0;
    }
    case EZ_FLASH_OMEGA: {
      return _EZFO_readSectors(sector, count, destination);
    }
    default:
      return false;
  }
}
