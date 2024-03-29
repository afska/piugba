#ifndef GBA_SPRITE_ENGINE_PALETTE_MANAGER_H
#define GBA_SPRITE_ENGINE_PALETTE_MANAGER_H

#pragma GCC system_header

#include <libgba-sprite-engine/gba/tonc_memmap.h>
#include <libgba-sprite-engine/gba/tonc_types.h>

#define PALETTE_BANK_SIZE 16
#define PALETTE_MAX_SIZE 256

int getBits(int number, int k, int p);

class PaletteManager {
 public:
  PaletteManager();
  PaletteManager(const COLOR paletteData[])
      : PaletteManager(paletteData, PALETTE_MAX_SIZE) {}
  PaletteManager(const COLOR paletteData[], int size);

  void persist();
  void persistToBank(int bank);
  COLOR change(int bank, int index, COLOR newColor);
  COLOR get(int bank, int index) { return paletteBank()[bank][index]; }

  static COLOR color(u32 r, u32 g, u32 b);

 protected:
  const COLOR* data;
  int size;

  virtual void* paletteAddress() = 0;
  virtual PALBANK* paletteBank() = 0;
};

class BackgroundPaletteManager : public PaletteManager {
 protected:
  void* paletteAddress() override { return pal_bg_mem; }

  PALBANK* paletteBank() override { return pal_bg_bank; }

 public:
  BackgroundPaletteManager() : PaletteManager() {}
  BackgroundPaletteManager(const COLOR paletteData[])
      : PaletteManager(paletteData) {}
  BackgroundPaletteManager(const COLOR paletteData[], int size)
      : PaletteManager(paletteData, size) {}
};

class ForegroundPaletteManager : public PaletteManager {
 protected:
  void* paletteAddress() override { return pal_obj_mem; }

  PALBANK* paletteBank() override { return pal_obj_bank; }

 public:
  ForegroundPaletteManager() : PaletteManager() {}
  ForegroundPaletteManager(const COLOR paletteData[])
      : PaletteManager(paletteData) {}
  ForegroundPaletteManager(const COLOR paletteData[], int size)
      : PaletteManager(paletteData, size) {}
};

#endif  // GBA_SPRITE_ENGINE_PALETTE_MANAGER_H
