#!/bin/bash

SOURCE="src/data"
DESTINATION="content/compiled"

cd "$SOURCE"
rm $DESTINATION/*.h $DESTINATION/*.c
grit \
  spr_arrows.bmp \
  spr_combo.bmp \
  spr_numbers.bmp \
  spr_feedback.bmp \
  spr_lifebar.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_song.c
mv *.h "$DESTINATION"
mv *.c "$DESTINATION"
grit \
  spr_arrows.bmp \
  spr_combo.bmp \
  spr_numbers.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_selection.c
mv palette_selection.h "$DESTINATION"
mv palette_selection.c "$DESTINATION"
rm *.h *.c
