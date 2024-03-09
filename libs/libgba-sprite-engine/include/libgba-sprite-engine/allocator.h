#ifndef GBA_SPRITE_ENGINE_ALLOCATOR_H
#define GBA_SPRITE_ENGINE_ALLOCATOR_H

#pragma GCC system_header

#include <libgba-sprite-engine/gba/tonc_memmap.h>
#include <libgba-sprite-engine/gba/tonc_types.h>
#include <vector>

#define MEM_OBJ_VRAM_BASE (MEM_VRAM + VRAM_BG_SIZE)

class AllocatedData {
 public:
  void* pointer() const { return (void*)currentAddress; };
  u32 size, currentAddress, baseAddress;

  const int getTileLocation() const {
    return (currentAddress - baseAddress) >> 5;
  }

  AllocatedData(u32 address, u32 size, u32 base)
      : currentAddress(address), size(size), baseAddress(base) {}
};

class Allocator {
 public:
  static void free();
  static u32 getCurrentSpriteIndex() { return currentSpriteIndex; }
  static int getAllocatedSprites() { return allocatedSprites.size(); }
  static AllocatedData& allocateObjectTiles(u32 size);
  static std::vector<AllocatedData> allocatedSprites;

 private:
  Allocator() = delete;
  Allocator(Allocator& other) = delete;
  Allocator(Allocator&& other) = delete;

  static u32 currentSpriteIndex;
};

#endif  // GBA_SPRITE_ENGINE_ALLOCATOR_H
