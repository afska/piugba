#ifndef AUDIO_STORE_H
#define AUDIO_STORE_H

#include <stdbool.h>
#include <stdlib.h>

#define AUDIOS_FOLDER_NAME "/piuGBA_audios/"

bool audio_store_load(char* audioPath);
bool audio_store_read(void* buffer);
unsigned int audio_store_len();

#endif  // AUDIO_STORE_H
