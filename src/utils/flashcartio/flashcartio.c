#include "flashcartio.h"

#include "everdrivegbax5/bios.h"
#include "everdrivegbax5/disk.h"

ActiveFlashcart active_flashcart = NO_FLASHCART;

bool flashcartio_activate(void) {
  // Everdrive GBA X5
  if (bi_init_sd_only()) {
    bi_init();
    bool success = diskInit() == 0;
    bi_lock_regs();
    if (!success)
      return false;

    active_flashcart = EVERDRIVE_GBA_X5;
    return true;
  }

  return false;
}

bool flashcartio_read_sector(u32 sector, u8* destination, u16 count) {
  switch (active_flashcart) {
    case EVERDRIVE_GBA_X5: {
      bi_unlock_regs();
      bool success = diskRead(sector, destination, count) == 0;
      bi_lock_regs();
      return success;
    }
    default:
      return false;
  }
}
