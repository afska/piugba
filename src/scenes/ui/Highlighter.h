#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

class Highlighter {
 public:
  Highlighter(u8 id);

  void initialize();
  void select(u8 option);
  u8 getSelectedItem();

 private:
  u8 id;
  u8 selectedItem = 0;

  void loadPalette();
  void loadTiles();
  void loadMap();
};

#endif  // HIGHLIGHTER_H
