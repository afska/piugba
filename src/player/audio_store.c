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

void unload() {
  if (!hasLoaded)
    return;

  f_close(&file);
  hasLoaded = false;
}

bool audio_store_load(char* audioPath) {
  if (PlaybackState.fatfs == NULL)
    return false;

  unload();
  char fileName[64];
  strcpy(fileName, AUDIOS_FOLDER_NAME);
  strcat(fileName, audioPath);

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

  return true;
}

unsigned int audio_store_len() {
  return hasLoaded ? f_size(&file) : 0;
}

bool audio_store_read(void* buffer) {
  if (!hasLoaded)
    return false;

  unsigned int readBytes;
  return f_read(&file, buffer, AUDIO_SIZE_FRAME, &readBytes) == 0;
}

// TODO: SEEK
// bool VideoStore::seek(u32 msecs) {
//   frame = MATH_fracumul(msecs, FRACUMUL_MS_TO_FRAME_AT_30FPS) -
//           SGN(videoOffset) *
//               MATH_fracumul(ABS(videoOffset), FRACUMUL_MS_TO_FRAME_AT_30FPS);
//   frameLatch = false;
//   cursor = frame > 0 ? frame * VIDEO_SIZE_FRAME / VIDEO_SECTOR : 0;
//   memoryCursor = 0;

//   if (f_lseek(&file, cursor * VIDEO_SECTOR) > 0) {
//     unload();
//     return false;
//   }

//   return true;
// }
