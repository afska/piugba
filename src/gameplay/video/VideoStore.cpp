#include "VideoStore.h"
#include "gameplay/models/Song.h"
#include "gameplay/save/SaveFile.h"

extern "C" {
#include "utils/flashcartio/flashcartio.h"
}

#define SECTOR 512
#define CLMT_ENTRIES 1024
#define SIZE_CLMT (CLMT_ENTRIES * sizeof(u32))
#define SIZE_PALETTE 512
#define SIZE_MAP 2048
#define SIZE_TILES 38912
#define SIZE_HALF_FRAME (SIZE_PALETTE + SIZE_MAP + SIZE_TILES / 2)
#define REQUIRED_MEMORY (SIZE_CLMT + SIZE_HALF_FRAME)

static FATFS fatfs;
static FIL file;

bool VideoStore::isActivating() {
  return SAVEFILE_read8(SRAM->adminSettings.isInitializingVideos);
}

VideoStore::State VideoStore::activate() {
  state = OFF;
  SAVEFILE_write8(SRAM->adminSettings.isInitializingVideos, true);

  if (!flashcartio_activate()) {
    state = NO_SUPPORTED_FLASHCART;
    return state;
  }

  if (f_mount(&fatfs, "", 1) > 0) {
    state = MOUNT_ERROR;
    return state;
  }

  state = ACTIVE;
  return state;
}

bool VideoStore::load(std::string videoPath) {
  if (state != ACTIVE)
    return false;
  memory = getSecondaryMemory(REQUIRED_MEMORY);
  if (memory == NULL)
    return false;

  cursor = 2;  // TODO: PARSE HEADER
  f_open(&file, videoPath.c_str(), FA_READ);
  file.cltbl = (DWORD*)memory;
  file.cltbl[0] = CLMT_ENTRIES;
  f_lseek(&file, CREATE_LINKMAP);
  f_lseek(&file, cursor * SECTOR);
  // TODO: ERROR CHECK

  return true;
}

bool VideoStore::read(u8* buffer, u32 sectors) {
  u32 readBytes;
  bool success = f_read(&file, buffer, sectors * SECTOR, &readBytes) == 0;
  cursor += sectors;
  return success;
}
