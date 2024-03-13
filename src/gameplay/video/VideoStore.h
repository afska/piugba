#ifndef VIDEO_STORE_H
#define VIDEO_STORE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <string>

#define VIDEOS_FOLDER_NAME "/piuGBA_videos/"
#define VIDEO_SIZE_PALETTE 512
#define VIDEO_SIZE_MAP 2048
#define VIDEO_SIZE_TILES 38912
#define VIDEO_SECTOR 512
#define VIDEO_SIZE_FRAME \
  (VIDEO_SIZE_PALETTE + VIDEO_SIZE_MAP + VIDEO_SIZE_TILES)

class VideoStore {
 public:
  enum State { OFF, NO_SUPPORTED_FLASHCART, MOUNT_ERROR, ACTIVE };
  enum LoadResult { OK, NO_FILE, ERROR };

  bool isActive() { return state == ACTIVE; }
  bool canRead() { return frame >= 0; }
  bool isPreRead() { return !frameLatch; }
  void advance(bool newFrame = false) {
    frameLatch = !frameLatch;
    frame += newFrame;
  }

  bool isEnabled();
  void disable();
  bool isActivating();
  State activate();
  LoadResult load(std::string videoPath, int videoOffset);
  void unload();

  bool seek(u32 msecs);
  bool preRead();
  bool endRead(u8* buffer, u32 sectors);

 private:
  State state = OFF;
  u8* memory = NULL;
  u32 cursor = 0;
  u32 memoryCursor = 0;
  bool isPlaying = false;
  bool frameLatch = false;
  int frame = 0;
  int videoOffset = 0;
};

extern VideoStore* videoStore;

#endif  // VIDEO_STORE_H
