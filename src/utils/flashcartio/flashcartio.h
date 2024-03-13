#ifndef FLASHCARTIO_H
#define FLASHCARTIO_H

#include <stdbool.h>
#include "fatfs/ff.h"

typedef enum { NO_FLASHCART, EVERDRIVE_GBA_X5 } ActiveFlashcart;
typedef enum {
  FLASHCART_ACTIVATED,
  NO_FLASHCART_FOUND,
  FLASHCART_ACTIVATION_FAILED
} ActivationResult;
extern volatile bool IS_FLASHCART_UNLOCKED;  // [!]

extern ActiveFlashcart active_flashcart;

ActivationResult flashcartio_activate(void);
bool flashcartio_read_sector(unsigned int sector,
                             unsigned char* destination,
                             unsigned short count);
void flashcartio_lock();  // [!]

#endif  // FLASHCARTIO_H
