#ifndef GBA_SPRITE_ENGINE_SPRITE_MANAGER_H
#define GBA_SPRITE_ENGINE_SPRITE_MANAGER_H

#pragma GCC system_header

#include <libgba-sprite-engine/gba/tonc_types.h>

#include <vector>

#include "sprite.h"

class SpriteManager {
 public:
  int getSpriteSize() { return sprites.size(); }

  void hideAll();
  void add(Sprite* sprite);
  void set(std::vector<Sprite*> sprites);
  void persist();  // copies over image and palette data to VRAM, modifies
                   // sprite OAM indiches

  inline void
  render() {  // copies over OAM buffer to OAM RAM, called in game loop
    // WARNING - This is called every time in the main update loop; keep amount
    // of instructions as minimal as possible in here!
    copyOverSpriteOAMToVRAM();
  }

 private:
  bool initialized;
  std::vector<Sprite*> sprites;

  void copyOverSpriteOAMToVRAM();

  void copyOverImageDataToVRAM(Sprite* s);
  void copyOverImageDataToVRAM();
};

#endif  // GBA_SPRITE_ENGINE_SPRITE_MANAGER_H
