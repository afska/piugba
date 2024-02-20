#include "VideoStore.h"
#include "gameplay/models/Song.h"
#include "gameplay/save/SaveFile.h"
#include "utils/SceneUtils.h"

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

bool VideoStore::isEnabled() {
  return SAVEFILE_read8(SRAM->adminSettings.backgroundVideos);
}

bool VideoStore::isActivating() {
  return SAVEFILE_read8(SRAM->adminSettings.isActivatingVideos);
}

void VideoStore::enable() {
  SAVEFILE_write8(SRAM->adminSettings.backgroundVideos, true);
  SCENE_softReset();
}

void VideoStore::disable() {
  SAVEFILE_write8(SRAM->adminSettings.backgroundVideos, false);
  SAVEFILE_write8(SRAM->adminSettings.isActivatingVideos, false);
}

VideoStore::State VideoStore::activate() {
  state = OFF;
  SAVEFILE_write8(SRAM->adminSettings.isActivatingVideos, true);

  if (!flashcartio_activate())
    state = NO_SUPPORTED_FLASHCART;
  else if (f_mount(&fatfs, "", 1) > 0)
    state = MOUNT_ERROR;
  else
    state = ACTIVE;

  SAVEFILE_write8(SRAM->adminSettings.isActivatingVideos, false);
  return state;
}

bool VideoStore::load(std::string videoPath) {
  if (state != ACTIVE)
    return false;
  memory = getSecondaryMemory(REQUIRED_MEMORY);
  if (memory == NULL)
    return false;

  cursor = 2;  // TODO: PARSE HEADER
  if (f_open(&file, videoPath.c_str(), FA_READ) > 0)
    return false;
  file.cltbl = (DWORD*)memory;
  file.cltbl[0] = CLMT_ENTRIES;
  if (f_lseek(&file, CREATE_LINKMAP) > 0)
    return false;
  if (f_lseek(&file, cursor * SECTOR) > 0)
    return false;

  return true;
}

bool VideoStore::read(u8* buffer, u32 sectors) {
  u32 readBytes;
  bool success = f_read(&file, buffer, sectors * SECTOR, &readBytes) == 0;
  cursor += sectors;
  return success;
}
