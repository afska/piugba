#ifndef FLASHCARTIO_H
#define FLASHCARTIO_H

#include <stdbool.h>
#include "fatfs/ff.h"

typedef enum { NO_FLASHCART, EVERDRIVE_GBA_X5, EZ_FLASH_OMEGA } ActiveFlashcart;
typedef enum {
  FLASHCART_ACTIVATED,
  NO_FLASHCART_FOUND,
  FLASHCART_ACTIVATION_FAILED
} ActivationResult;  // [!]

#define CAN_USE_BG_VIDEO_WITH_GSM (false)     // [!]
#define CAN_USE_HQ_AUDIO_DURING_MENUS (true)  // [!]

extern ActiveFlashcart active_flashcart;
extern volatile bool flashcartio_is_reading;
extern volatile bool flashcartio_needs_reset;     // [!]
extern void (*flashcartio_reset_callback)(void);  // [!]

ActivationResult flashcartio_activate(void);
bool flashcartio_read_sector(unsigned int sector,
                             unsigned char* destination,
                             unsigned short count);

#endif  // FLASHCARTIO_H
