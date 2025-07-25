#ifndef AUDIO_STORE_H
#define AUDIO_STORE_H

#include <stdbool.h>
#include <stdlib.h>

#define AUDIOS_FOLDER_NAME_ABS "/piuGBA_audios/"
#define AUDIOS_FOLDER_NAME_REL "piuGBA_audios/"

bool audio_store_load(char* audioPath);
bool audio_store_read(void* buffer, int size);
bool audio_store_seek(unsigned int offset);
unsigned int audio_store_len();

#endif  // AUDIO_STORE_H
