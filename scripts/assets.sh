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
  spr_multipliers.bmp \
  spr_grades_mini.bmp \
  spr_channels.bmp \
  spr_lock.bmp \
  spr_level.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_selection.c
mv \
  spr_difficulties.h \
  spr_difficulties.c \
  spr_of.h \
  spr_of.c \
  spr_multipliers.h \
  spr_multipliers.c \
  spr_grades_mini.h \
  spr_grades_mini.c \
  spr_channels.h \
  spr_channels.c \
  spr_lock.h \
  spr_lock.c \
  spr_level.h \
  spr_level.c \
  palette_selection.* \
  "$DESTINATION"

# StartScene
grit \
  spr_arrows.bmp \
  spr_buttons.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_start.c
mv \
  spr_buttons.h \
  spr_buttons.c \
  palette_start.* \
  "$DESTINATION"

# ControlsScene
grit \
  spr_arrows.bmp \
  spr_instructors.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_controls.c
mv \
  spr_instructors.h \
  spr_instructors.c \
  palette_controls.* \
  "$DESTINATION"

# StageBreakScene
grit \
  spr_arrows.bmp \
  spr_instructors.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_break.c
mv \
  palette_break.* \
  "$DESTINATION"

# DanceGradeScene
grit \
  spr_numbers_mini.bmp \
  spr_grade_*.bmp \
  -ftc -pS -gB8 -gT ff00ff -O palette_grade.c
mv \
  spr_numbers_mini.h \
  spr_numbers_mini.c \
  spr_grade_*.h \
  spr_grade_*.c \
  palette_grade.* \
  "$DESTINATION"

# [Cleanup]
rm *.h *.c
