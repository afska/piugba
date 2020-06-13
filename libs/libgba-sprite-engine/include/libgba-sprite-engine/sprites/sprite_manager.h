//
// Created by Wouter Groeneveld on 26/07/18.
//

#ifndef GBA_SPRITE_ENGINE_SPRITE_MANAGER_H
#define GBA_SPRITE_ENGINE_SPRITE_MANAGER_H

#pragma GCC system_header

#include <libgba-sprite-engine/gba/tonc_types.h>

#include <vector>

#include "sprite.h"

class SpriteManager {
 private:
  bool initialized;
  std::vector<Sprite*> sprites;

  inline void copyOverSpriteOAMToVRAM() {
    int i = 0;
    // int affineIndex = 0;

    for (auto sprite : this->sprites) {
      if (sprite->enabled) {
        sprite->update();
        oam_mem[i] = sprite->oam;

        // auto affine = dynamic_cast<AffineSprite*>(sprite);
        // if(affine) {
        //     // WHY warning: can't do this: obj_aff_mem[affineIndex] =
        //     *affineShadow;
        //     // because that would override OAM also! only want to set
        //     non-overlapping affine attribs
        //     affine->setTransformationMatrix(&obj_aff_mem[affineIndex]);
        //     affine->setAffineIndex(affineIndex);
        //     affineIndex++;
        // }
      }

      i++;
    }
  }

  void copyOverImageDataToVRAM(Sprite* s);
  void copyOverImageDataToVRAM();

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
};

#endif  // GBA_SPRITE_ENGINE_SPRITE_MANAGER_H
