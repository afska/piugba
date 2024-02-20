#ifndef VIDEO_STORE_H
#define VIDEO_STORE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include <string>

class VideoStore {
 public:
  enum State { OFF, NO_SUPPORTED_FLASHCART, MOUNT_ERROR, ACTIVE };

  bool isEnabled();
  void enable();
  void disable();
  bool isActivating();
  State activate();
  bool load(std::string videoPath);
  bool read(u8* buffer, u32 sectors);

 private:
  State state = OFF;
  u8* memory = NULL;
  u32 cursor = 0;
};

extern VideoStore* videoStore;

#endif  // VIDEO_STORE_H
