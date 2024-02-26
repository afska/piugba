#ifndef FLASHCARTIO_H
#define FLASHCARTIO_H

#include <stdbool.h>
#include "fatfs/ff.h"

typedef enum { NO_FLASHCART, EVERDRIVE_GBA_X5 } ActiveFlashcart;

extern bool IS_FLASHCART_UNLOCKED;  // [!]

extern ActiveFlashcart active_flashcart;

bool flashcartio_activate(void);
bool flashcartio_read_sector(unsigned int sector,
                             unsigned char* destination,
                             unsigned short count);
void flashcartio_lock();  // [!]

#endif  // FLASHCARTIO_H
