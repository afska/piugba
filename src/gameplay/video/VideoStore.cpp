#include "VideoStore.h"

#include <libgba-sprite-engine/gba/tonc_math.h>

#include "gameplay/models/Song.h"
#include "gameplay/save/SaveFile.h"
#include "objects/ArrowInfo.h"

extern "C" {
#include "utils/flashcartio/flashcartio.h"
}

#define VIDEO_SECTOR 512
#define CLMT_ENTRIES 1024
#define SIZE_CLMT (CLMT_ENTRIES * sizeof(u32))
#define SIZE_HALF_FRAME \
  (VIDEO_SIZE_PALETTE + VIDEO_SIZE_MAP + VIDEO_SIZE_TILES / 2)
#define REQUIRED_MEMORY (SIZE_CLMT + SIZE_HALF_FRAME)

static FATFS fatfs;
static FIL file;

bool VideoStore::isEnabled() {
  return SAVEFILE_read8(SRAM->adminSettings.backgroundVideos);
}

bool VideoStore::isActivating() {
  return static_cast<BackgroundVideosOpts>(SAVEFILE_read8(
             SRAM->adminSettings.backgroundVideos)) == dACTIVATING;
}

void VideoStore::disable() {
  SAVEFILE_write8(SRAM->adminSettings.backgroundVideos,
                  BackgroundVideosOpts::dOFF);
}

VideoStore::State VideoStore::activate() {
  state = OFF;
  SAVEFILE_write8(SRAM->adminSettings.backgroundVideos,
                  BackgroundVideosOpts::dACTIVATING);

  if (!flashcartio_activate()) {
    disable();
    return (state = NO_SUPPORTED_FLASHCART);
  }

  if (f_mount(&fatfs, "", 1) > 0) {
    disable();
    return (state = MOUNT_ERROR);
  }

  state = ACTIVE;
  SAVEFILE_write8(SRAM->adminSettings.backgroundVideos,
                  BackgroundVideosOpts::dACTIVE);
  return state;
}

bool VideoStore::load(std::string videoPath) {
  if (state != ACTIVE)
    return false;
  memory = getSecondaryMemory(REQUIRED_MEMORY);
  if (memory == NULL)
    return false;

  if (f_open(&file, (VIDEOS_FOLDER_NAME + videoPath).c_str(), FA_READ) > 0)
    return false;

  isPlaying = true;
  frameLatch = false;
  cursor = 2;  // TODO: PARSE HEADER
  memoryCursor = 0;

  file.cltbl = (DWORD*)memory;
  file.cltbl[0] = CLMT_ENTRIES;
  if (f_lseek(&file, CREATE_LINKMAP) > 0) {
    unload();
    return false;
  }

  if (f_lseek(&file, cursor * VIDEO_SECTOR) > 0) {
    unload();
    return false;
  }

  return true;
}

void VideoStore::unload() {
  if (!isPlaying)
    return;

  f_close(&file);
  isPlaying = false;
}

CODE_IWRAM bool VideoStore::preRead() {
  u32 readBytes;
  bool success =
      f_read(&file, memory + SIZE_CLMT, SIZE_HALF_FRAME, &readBytes) == 0;
  cursor += SIZE_HALF_FRAME / VIDEO_SECTOR;
  memoryCursor = 0;
  return success;
}

CODE_IWRAM bool VideoStore::endRead(u8* buffer, u32 sectors) {
  u32 readFromMemory = 0;

  while (sectors > 0) {
    u32 availableMemory = SIZE_HALF_FRAME - memoryCursor;
    u32 pendingBytes = sectors * VIDEO_SECTOR;

    if (availableMemory > 0) {
      u32 readableBytes = min(availableMemory, pendingBytes);
      dma_cpy(buffer, memory + SIZE_CLMT + memoryCursor, readableBytes / 4, 3,
              DMA_CPY32);  // TODO: DMA3Copy?
      memoryCursor += readableBytes;
      readFromMemory += readableBytes;
      sectors -= readableBytes / VIDEO_SECTOR;
    } else {
      u32 readBytes;
      if (f_read(&file, buffer + readFromMemory, pendingBytes, &readBytes) > 0)
        return false;
      cursor += sectors;
      sectors = 0;
    }
  }

  return true;
}
