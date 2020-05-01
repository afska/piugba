#!/bin/bash

SOURCE="src/data"
DESTINATION="content/_compiled_sprites"

cd "$SOURCE"
rm $DESTINATION/*.h $DESTINATION/*.c
grit \
  spr_arrows.bmp \
  spr_combo.bmp \
  spr_numbers.bmp \
  spr_feedback.bmp \
  spr_lifebar.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_song.c
mv *.h *.c "$DESTINATION"
grit \
  spr_arrows.bmp \
  spr_combo.bmp \
  spr_numbers.bmp \
  spr_difficulties.bmp \
  spr_of.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_selection.c
mv \
  spr_difficulties.h \
  spr_difficulties.c \
  spr_of.h \
  spr_of.c \
  palette_selection.* \
  "$DESTINATION"
rm *.h *.c
