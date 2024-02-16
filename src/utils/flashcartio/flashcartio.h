#ifndef FLASHCARTIO_H
#define FLASHCARTIO_H

#include <tonc_core.h>

typedef enum { NONE, EVERDRIVE_GBA_X5 } ActiveFlashcart;

extern ActiveFlashcart active_flashcart;

bool flashcartio_activate(void);
bool flashcartio_read_sector(u32 sector, u8* destination, u16 count);

#endif  // FLASHCARTIO_H
