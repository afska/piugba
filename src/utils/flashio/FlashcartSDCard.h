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

  ActiveFlashcart getActiveFlashcart() { return activeFlashcart; }

  bool activate();
  bool read(u32 sector, u8* destination, u16 count);

 private:
  ActiveFlashcart activeFlashcart = NONE;
};

extern FlashcartSDCard* flashcartSDCard;

#endif  // FLASHCART_SD_CARD_H
