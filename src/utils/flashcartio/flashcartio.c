#include "flashcartio.h"
#include "everdrivegbax5/bios.h"
#include "everdrivegbax5/disk.h"

ActiveFlashcart active_flashcart = NONE;

bool flashcartio_activate(void) {
  bi_init_sd_only();
  if (diskInit() == 0) {
    active_flashcart = EVERDRIVE_GBA_X5;
    return true;
  }

  return false;
}

bool flashcartio_read_sector(u32 sector, u8* destination, u16 count) {
  switch (active_flashcart) {
    case EVERDRIVE_GBA_X5: {
      return diskRead(sector, destination, count) == 0;
    }
    default:
      return false;
  }
}
