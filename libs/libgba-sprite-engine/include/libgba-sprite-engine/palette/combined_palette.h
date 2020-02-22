//
// Created by Wouter Groeneveld on 05/08/18.
//

#ifndef GBA_SPRITE_ENGINE_COMBINED_PALETTE_H
#define GBA_SPRITE_ENGINE_COMBINED_PALETTE_H

class PaletteManager;

class CombinedPalette {
private:
    // WHY use references here? lifetimes not bound, not owned by CombinedPalette
    PaletteManager& palette1;
    PaletteManager& palette2;

    void increaseBrightness(PaletteManager& palette, int bank, int index, u32 intensity);
public:
    CombinedPalette(PaletteManager& one, PaletteManager& two) : palette1(one), palette2(two) {}
    CombinedPalette(const CombinedPalette& other) = delete;

    void increaseBrightness(u32 intensity);
};


#endif //GBA_SPRITE_ENGINE_COMBINED_PALETTE_H
