#include "audio_store.h"

#include <string.h>
#include "PlaybackState.h"
#include "utils/flashcartio/flashcartio.h"

#define DATA_EWRAM __attribute__((section(".ewram")))
#define CLMT_ENTRIES 256
#define AUDIO_SIZE_FRAME 608

DATA_EWRAM static unsigned int clmt[CLMT_ENTRIES];
DATA_EWRAM static FIL file;
DATA_EWRAM static bool hasLoaded = false;
DATA_EWRAM static char currentAudioPath[MAX_PATH_LENGTH];

void unload() {
  if (!hasLoaded)
    return;

  if (!isEmulatorFsEnabled())
    f_close(&file);

  hasLoaded = false;
}

bool audio_store_load(char* audioPath) {
  if (PlaybackState.fatfs == NULL)
    return false;

  unload();
  char fileName[MAX_PATH_LENGTH];
  strcpy(fileName, isEmulatorFsEnabled() ? AUDIOS_FOLDER_NAME_REL
                                         : AUDIOS_FOLDER_NAME_ABS);
  strcat(fileName, audioPath);

  for (unsigned i = 0; i < MAX_PATH_LENGTH; i++)
    currentAudioPath[i] = fileName[i];

  if (isEmulatorFsEnabled()) {
    unsigned char test[2];
    if (readFile(fileName, 0, 1, test) == -1)
      return false;

    hasLoaded = true;
    file.sect = 0;
  } else {
    FRESULT result = f_open(&file, fileName, FA_READ);
    if (result > 0)
      return false;

    hasLoaded = true;

    file.cltbl = (DWORD*)clmt;
    file.cltbl[0] = CLMT_ENTRIES;
    if (f_lseek(&file, CREATE_LINKMAP) > 0) {
      unload();
      return false;
    }
  }

  return true;
}

bool audio_store_read(void* buffer, int size) {
  if (!hasLoaded)
    return false;

  unsigned int readBytes;
  if (isEmulatorFsEnabled()) {
    int count = readFile(currentAudioPath, file.sect, size, buffer);
    bool success = count > -1;
    if (success) {
      file.sect += count;
      readBytes = count;
    } else
      readBytes = 0;
    return success;
  } else {
    return f_read(&file, buffer, size, &readBytes) == 0;
  }
}

bool audio_store_seek(unsigned int offset) {
  if (!hasLoaded)
    return false;

  if (isEmulatorFsEnabled()) {
    file.sect = offset;
    return true;
  } else {
    return f_lseek(&file, offset) == 0;
  }
}

unsigned int audio_store_len() {
  if (!hasLoaded)
    return 0;

  if (isEmulatorFsEnabled()) {
    int size = getFileSize(currentAudioPath);
    return size > -1 ? size : 0;
  } else {
    return f_size(&file);
  }
}
