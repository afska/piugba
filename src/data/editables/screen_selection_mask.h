
//{{BLOCK(screen_selection_mask)

//======================================================================
//
//	screen_selection_mask, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 1025 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 65600 + 2048 = 68160
//
//	Time-stamp: 2020-04-19, 23:41:11
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.15
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_SCREEN_SELECTION_MASK_H
#define GRIT_SCREEN_SELECTION_MASK_H

#define screen_selection_maskTilesLen 65600
extern const unsigned int screen_selection_maskTiles[16400];

#define screen_selection_maskMapLen 2048
extern const unsigned short screen_selection_maskMap[1024];

#define screen_selection_maskPalLen 512
extern const unsigned short screen_selection_maskPal[256];

#endif // GRIT_SCREEN_SELECTION_MASK_H

//}}BLOCK(screen_selection_mask)
