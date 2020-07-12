#!/bin/bash

SOURCE="src/data"
DESTINATION="content/_compiled_sprites"

cd "$SOURCE"

# [Setup]
mkdir -p "$DESTINATION"
rm $DESTINATION/*.h $DESTINATION/*.c

# SongScene
grit \
  spr_arrows.bmp \
  spr_combo.bmp \
  spr_numbers.bmp \
  spr_feedback.bmp \
  spr_lifebar.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_song.c
mv *.h *.c "$DESTINATION"

# SelectionScene
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

# DanceGradeScene
grit \
  spr_grades.bmp \
  spr_numbers_mini.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_grade.c
mv \
  spr_grades.h \
  spr_grades.c \
  spr_numbers_mini.h \
  spr_numbers_mini.c \
  palette_grade.* \
  "$DESTINATION"

# [Cleanup]
rm *.h *.c
