#include "flashcartio.h"

#include "everdrivegbax5/bios.h"
#include "everdrivegbax5/disk.h"

volatile bool IS_FLASHCART_UNLOCKED = false;

ActiveFlashcart active_flashcart = NO_FLASHCART;

ActivationResult flashcartio_activate(void) {
  // Everdrive GBA X5
  if (bi_init_sd_only()) {
    bi_init();
    bool success = diskInit() == 0;
    bi_lock_regs();
    if (!success)
      return FLASHCART_ACTIVATION_FAILED;

    active_flashcart = EVERDRIVE_GBA_X5;
    return FLASHCART_ACTIVATED;
  }

  return NO_FLASHCART_FOUND;
}

bool flashcartio_read_sector(u32 sector, u8* destination, u16 count) {
  switch (active_flashcart) {
    case EVERDRIVE_GBA_X5: {
      IS_FLASHCART_UNLOCKED = true;
      bi_unlock_regs();
      bool success = diskRead(sector, destination, count) == 0;
      bi_lock_regs();
      IS_FLASHCART_UNLOCKED = false;
      return success;
    }
    default:
      return false;
  }
}

// [!]
void flashcartio_lock() {
  bi_lock_regs();
  IS_FLASHCART_UNLOCKED = false;
}
