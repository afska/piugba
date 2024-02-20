#ifndef VIDEO_STORE_H
#define VIDEO_STORE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <string>

#define VIDEOS_FOLDER_NAME "/piuGBA_videos/"

class VideoStore {
 public:
  enum State { OFF, NO_SUPPORTED_FLASHCART, MOUNT_ERROR, ACTIVE };

  bool isActive() { return state == ACTIVE; }
  bool isPreRead() { return !frameLatch; }

  bool isEnabled();
  void enable();
  void disable();
  bool isActivating();
  State activate();
  bool load(std::string videoPath);
  void unload();

  bool preRead();
  bool endRead(u8* buffer, u32 sectors, u32 frameCursor);

 private:
  State state = OFF;
  u8* memory = NULL;
  u32 cursor = 0;
  bool isPlaying = false;
  bool frameLatch = false;
};

extern VideoStore* videoStore;

#endif  // VIDEO_STORE_H
