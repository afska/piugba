#ifndef FLASHCART_SD_CARD_H
#define FLASHCART_SD_CARD_H

#include <tonc_core.h>

extern "C" {
#include "everdrivegbax5/bios.h"
#include "everdrivegbax5/disk.h"
}

class FlashcartSDCard {
 public:
  enum ActiveFlashcart { NONE, EVERDRIVE_GBA_X5 };

  bool activate() {
    bi_init_sd_only();
    if (diskInit() == 0) {
      activeFlashcart = EVERDRIVE_GBA_X5;
      return true;
    }

    return false;
  }

  ActiveFlashcart getActiveFlashcart() { return activeFlashcart; }

  bool read(u32 sector, u8* destination, u16 count) {
    switch (activeFlashcart) {
      case EVERDRIVE_GBA_X5: {
        return diskRead(sector, destination, count) == 0;
      }
      default:
        return false;
    }
  }

 private:
  ActiveFlashcart activeFlashcart = NONE;
};

extern FlashcartSDCard* flashcartSDCard;

#endif  // FLASHCART_SD_CARD_H
