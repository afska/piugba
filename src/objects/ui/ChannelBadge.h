#ifndef CHANNEL_BADGE_H
#define CHANNEL_BADGE_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "gameplay/models/Song.h"

class ChannelBadge {
 public:
  ChannelBadge(u32 x, u32 y, bool reuseTiles);

  void setType(Channel type);
  void hide();

  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;

  u32 x;
  u32 y;
};

#endif  // CHANNEL_BADGE_H
