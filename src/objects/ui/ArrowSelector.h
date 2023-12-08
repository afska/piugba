#ifndef ARROW_SELECTOR_H
#define ARROW_SELECTOR_H

#include <libgba-sprite-engine/sprites/sprite.h>

#include "objects/ArrowHolder.h"
#include "objects/ArrowInfo.h"
#include "objects/base/InputHandler.h"

class ArrowSelector : public InputHandler {
 public:
  ArrowDirection direction;

  ArrowSelector(ArrowDirection direction,
                bool reuseTiles,
                bool reactive,
                bool canUseGBAStyle = true,
                bool isVertical = false);

  bool shouldFireEvent();

  void tick();
  inline Sprite* get() { return sprite.get(); }

 private:
  std::unique_ptr<Sprite> sprite;
  u32 startTile = 0;
  u32 endTile = 0;
  u32 idleFrame = 0;
  u32 pressedFrame = 0;
  u32 animationFrames = 0;
  ArrowFlip flip = ArrowFlip::NO_FLIP;
  bool reactive = true;
  bool canUseGBAStyle = true;
  bool isVertical = false;
  u32 globalLastPressFrame = 0;
  u32 currentLastPressFrame = 0;
  u32 autoFireSpeed = 0;

  inline bool isUsingGBAStyle() {
    return canUseGBAStyle && SAVEFILE_isUsingGBAStyle();
  }

  void setUpAltKeysIfNeeded();
};

#endif  // ARROW_SELECTOR_H
