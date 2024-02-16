#include "FlashcartSDCard.h"

bool FlashcartSDCard::activate() {
  bi_init_sd_only();
  if (diskInit() == 0) {
    activeFlashcart = EVERDRIVE_GBA_X5;
    return true;
  }

  return false;
}

bool FlashcartSDCard::read(u32 sector, u8* destination, u16 count) {
  switch (activeFlashcart) {
    case EVERDRIVE_GBA_X5: {
      return diskRead(sector, destination, count) == 0;
    }
    default:
      return false;
  }
}
